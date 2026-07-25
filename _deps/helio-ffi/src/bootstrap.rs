use std::num::NonZeroIsize;
use std::ptr;
use std::sync::Arc;
use raw_window_handle::{
    HasWindowHandle, HasDisplayHandle,
    Win32WindowHandle,
    WindowHandle, DisplayHandle,
    RawWindowHandle,
};
use helio::{required_wgpu_features, required_experimental_features, required_wgpu_limits};

struct Win32Window {
    hwnd: isize,
    hinstance: isize,
}

impl HasWindowHandle for Win32Window {
    fn window_handle(&self) -> Result<WindowHandle<'_>, raw_window_handle::HandleError> {
        let hwnd = NonZeroIsize::new(self.hwnd).expect("HWND must be non-null");
        let mut handle = Win32WindowHandle::new(hwnd);
        if let Some(hi) = NonZeroIsize::new(self.hinstance) {
            handle.hinstance = Some(hi);
        }
        Ok(unsafe { WindowHandle::borrow_raw(RawWindowHandle::Win32(handle)) })
    }
}

impl HasDisplayHandle for Win32Window {
    fn display_handle(&self) -> Result<DisplayHandle<'_>, raw_window_handle::HandleError> {
        Ok(DisplayHandle::windows())
    }
}

unsafe impl Send for Win32Window {}
unsafe impl Sync for Win32Window {}

struct BootstrapState {
    instance: wgpu::Instance,
    device: Arc<wgpu::Device>,
    queue: Arc<wgpu::Queue>,
    adapter: wgpu::Adapter,
    surface: Option<wgpu::Surface<'static>>,
    format: wgpu::TextureFormat,
    width: u32,
    height: u32,
}

static mut STATE: Option<BootstrapState> = None;

#[no_mangle]
pub unsafe extern "C" fn bootstrap_init(
    width: u32,
    height: u32,
    out_device: *mut *mut std::ffi::c_void,
    out_queue: *mut *mut std::ffi::c_void,
    out_debug_cam: *mut *mut std::ffi::c_void,
    out_cull_stats: *mut *mut std::ffi::c_void,
) -> bool {
    let instance = wgpu::Instance::new(wgpu::InstanceDescriptor {
        backends: wgpu::Backends::DX12,
        flags: wgpu::InstanceFlags::empty(),
        ..wgpu::InstanceDescriptor::new_without_display_handle()
    });

    let adapters = pollster::block_on(instance.enumerate_adapters(wgpu::Backends::DX12));
    let (adapter, adapter_name, adapter_type) = {
        let mut best = None;
        for adapter in adapters {
            let info = adapter.get_info();
            match info.device_type {
                wgpu::DeviceType::DiscreteGpu => { best = Some((adapter, info)); break; }
                wgpu::DeviceType::IntegratedGpu => { if best.is_none() { best = Some((adapter, info)); } }
                _ => {}
            }
        }
        match best {
            Some((a, info)) => {
                let name = info.name.clone();
                let dtype = info.device_type;
                print!("[helio] Adapter: {} ", name);
                match dtype {
                    wgpu::DeviceType::DiscreteGpu => print!("(Discrete GPU)"),
                    wgpu::DeviceType::IntegratedGpu => print!("(Integrated GPU)"),
                    _ => print!("(Other)"),
                }
                println!(" [{:?}]", info.backend);
                (a, name, dtype)
            }
            None => { return false; }
        }
    };

    let (device, queue) = match pollster::block_on(adapter.request_device(
        &wgpu::DeviceDescriptor {
            label: Some("Helio Device"),
            required_features: required_wgpu_features(adapter.features()),
            required_limits: required_wgpu_limits(adapter.limits()),
            experimental_features: required_experimental_features(adapter.features()),
            ..Default::default()
        },
    )) {
        Ok(dq) => dq,
        Err(_) => return false,
    };

    let device = Arc::new(device);
    let queue = Arc::new(queue);

    let debug_cam_buf = Box::new(device.create_buffer(&wgpu::BufferDescriptor {
        label: Some("debug_camera"),
        size: 256,
        usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
        mapped_at_creation: false,
    }));
    let cull_stats_buf = Box::new(device.create_buffer(&wgpu::BufferDescriptor {
        label: Some("cull_stats"),
        size: 64,
        usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::COPY_SRC,
        mapped_at_creation: false,
    }));

    ptr::write(out_device, Arc::into_raw(device.clone()) as *mut _);
    ptr::write(out_queue, Arc::into_raw(queue.clone()) as *mut _);
    ptr::write(out_debug_cam, Box::into_raw(debug_cam_buf) as *mut _);
    ptr::write(out_cull_stats, Box::into_raw(cull_stats_buf) as *mut _);

    STATE = Some(BootstrapState {
        instance,
        device,
        queue,
        adapter,
        surface: None,
        format: wgpu::TextureFormat::Rgba8UnormSrgb,
        width,
        height,
    });

    true
}

#[no_mangle]
pub unsafe extern "C" fn bootstrap_create_surface(hinstance: *mut std::ffi::c_void, hwnd: *mut std::ffi::c_void) -> bool {
    let state = match STATE.as_mut() {
        Some(s) => s,
        None => return false,
    };
    let raw_win = Win32Window {
        hwnd: hwnd as isize,
        hinstance: hinstance as isize,
    };
    let surface = match state.instance.create_surface(raw_win) {
        Ok(s) => s,
        Err(_) => return false,
    };
    let caps = surface.get_capabilities(&state.adapter);
    let fmt = caps.formats.iter().copied()
        .find(|f| *f == wgpu::TextureFormat::Rgba8UnormSrgb)
        .or_else(|| caps.formats.iter().copied().find(|f| f.is_srgb()))
        .unwrap_or(caps.formats[0]);
    surface.configure(&state.device, &wgpu::SurfaceConfiguration {
        usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
        format: fmt,
        width: state.width,
        height: state.height,
        present_mode: wgpu::PresentMode::Fifo,
        alpha_mode: wgpu::CompositeAlphaMode::Auto,
        view_formats: vec![],
        desired_maximum_frame_latency: 2,
        color_space: wgpu::SurfaceColorSpace::Auto,
    });
    state.format = fmt;
    state.surface = Some(surface);
    true
}

/// Acquire the next swapchain texture, render to it, and present.
/// Returns false if the surface was lost.
#[no_mangle]
pub unsafe extern "C" fn bootstrap_render_frame(renderer: *mut std::ffi::c_void, camera: *const std::ffi::c_void) -> bool {
    let state = match STATE.as_mut() {
        Some(s) => s,
        None => return false,
    };
    let surface = match state.surface.as_ref() {
        Some(s) => s,
        None => return false,
    };

    // Acquire with retry
    let surface_tex = loop {
        match surface.get_current_texture() {
            wgpu::CurrentSurfaceTexture::Success(t) | wgpu::CurrentSurfaceTexture::Suboptimal(t) => break t,
            wgpu::CurrentSurfaceTexture::Timeout => {
                state.device.poll(wgpu::PollType::Poll);
                continue;
            }
            _ => return false,
        }
    };
    let view = surface_tex.texture.create_view(&wgpu::TextureViewDescriptor::default());

    // Render
    let renderer = &mut *(renderer as *mut helio::Renderer);
    let camera = &*(camera as *const crate::types::HelioCameraDesc);
    if let Err(e) = renderer.render(&crate::camera::camera_from_desc(camera), &view) {
        let _ = e;
        return false;
    }

    // Present (drop SurfaceTexture)
    drop(surface_tex);
    true
}

#[no_mangle]
pub unsafe extern "C" fn bootstrap_poll(wait: bool) {
    if let Some(ref state) = STATE {
        if wait {
            let _ = state.device.poll(wgpu::PollType::Wait {
                submission_index: None,
                timeout: None,
            });
        } else {
            let _ = state.device.poll(wgpu::PollType::Poll);
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn bootstrap_shutdown() {
    STATE = None;
}
