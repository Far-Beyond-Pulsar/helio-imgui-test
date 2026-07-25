use std::sync::{Arc, Mutex};

use helio::{
    DebugDrawState, Renderer, RendererConfig, GiConfig, PerfOverlayMode,
};
use helio_core::RenderGraph;
use helio_default_graphs;

use crate::types::*;
use crate::camera::camera_from_desc;

fn wgpu_texture_format(f: u32) -> wgpu::TextureFormat {
    match f {
        0 => wgpu::TextureFormat::Rgba8Unorm,
        1 => wgpu::TextureFormat::Rgba8UnormSrgb,
        2 => wgpu::TextureFormat::Bgra8Unorm,
        3 => wgpu::TextureFormat::Bgra8UnormSrgb,
        4 => wgpu::TextureFormat::R8Unorm,
        5 => wgpu::TextureFormat::Rgba16Float,
        _ => wgpu::TextureFormat::Rgba8UnormSrgb,
    }
}

/// Create a new Renderer, consuming the Scene and wgpu resources.
///
/// `device` and `queue` are consumed (their refcounts are increased).
/// `debug_camera_buffer` and `cull_stats_buffer` are consumed (ownership
/// transfers into the renderer).  Callers should not free them after calling.
#[no_mangle]
pub unsafe extern "C" fn helio_renderer_new(
    device: HelioDevicePtr,
    queue: HelioQueuePtr,
    surface_format: u32,
    config: &HelioRendererConfig,
    scene: HelioScenePtr,
    debug_camera_buffer: HelioBufferPtr,
    cull_stats_buffer: HelioBufferPtr,
) -> HelioRendererPtr {
    let device = Arc::from_raw(device as *const wgpu::Device);
    let queue = Arc::from_raw(queue as *const wgpu::Queue);
    let fmt = wgpu_texture_format(surface_format);

    let gi = GiConfig {
        rc_radius: config.gi_rc_radius,
        rc_fade_margin: config.gi_fade_margin,
    };

    let rcfg = RendererConfig {
        width: config.width.max(1),
        height: config.height.max(1),
        surface_format: fmt,
        gi_config: gi,
        shadow_quality: match config.shadow_quality {
            0 => libhelio::ShadowQuality::Low,
            1 => libhelio::ShadowQuality::Medium,
            2 => libhelio::ShadowQuality::High,
            3 => libhelio::ShadowQuality::Ultra,
            _ => libhelio::ShadowQuality::Medium,
        },
        debug_mode: config.debug_mode,
        render_scale: config.render_scale.clamp(0.25, 1.0),
        perf_overlay_mode: match config.perf_overlay_mode {
            0 => PerfOverlayMode::Disabled,
            1 => PerfOverlayMode::PassOverdraw,
            2 => PerfOverlayMode::ShaderComplexity,
            3 => PerfOverlayMode::TileLightCount,
            4 => PerfOverlayMode::PassOutput,
            _ => PerfOverlayMode::Disabled,
        },
        shadow_atlas_size: config.shadow_atlas_size,
        shadow_face_capacity: config.shadow_face_capacity.clamp(1, 256),
        enable_ssr: true,
        enable_planar_reflections: false,
    };

    let scene_box = Box::from_raw(scene as *mut helio::Scene);
    let debug_state = Arc::new(Mutex::new(DebugDrawState::default()));
    let debug_cam_buf = Box::from_raw(debug_camera_buffer as *mut wgpu::Buffer);
    let cull_buf = Box::from_raw(cull_stats_buffer as *mut wgpu::Buffer);

    let w = config.width.max(1);
    let h = config.height.max(1);

    let mut graph = helio_default_graphs::build_default_graph(
        &device,
        &queue,
        &scene_box,
        rcfg,
        debug_state.clone(),
        &debug_cam_buf,
        &cull_buf,
        None,
    );

    let renderer = Renderer::new(
        device,
        queue,
        fmt,
        w,
        h,
        config.render_scale.clamp(0.25, 1.0),
        rcfg,
        *scene_box,
        graph,
        debug_state,
        *debug_cam_buf,
        *cull_buf,
    );

    Box::into_raw(Box::new(renderer)) as HelioRendererPtr
}

#[no_mangle]
pub unsafe extern "C" fn helio_renderer_destroy(renderer: HelioRendererPtr) {
    if !renderer.is_null() {
        drop(Box::from_raw(renderer as *mut Renderer));
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_renderer_render(
    renderer: HelioRendererPtr,
    camera: &HelioCameraDesc,
    target: HelioTextureViewPtr,
) -> HelioResult {
    let renderer = &mut *(renderer as *mut Renderer);
    let target = &*(target as *const wgpu::TextureView);
    match renderer.render(&camera_from_desc(camera), target) {
        Ok(()) => HelioResult::ok(),
        Err(e) => HelioResult::err(&e.to_string()),
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_renderer_scene(renderer: HelioRendererPtr) -> HelioScenePtr {
    let renderer = &mut *(renderer as *mut Renderer);
    renderer.scene_mut() as *mut helio::Scene as HelioScenePtr
}

#[no_mangle]
pub unsafe extern "C" fn helio_renderer_set_clear_color(
    renderer: HelioRendererPtr,
    color: &[f32; 4],
) {
    let renderer = &mut *(renderer as *mut Renderer);
    renderer.set_clear_color(*color);
}

#[no_mangle]
pub unsafe extern "C" fn helio_renderer_set_ambient(
    renderer: HelioRendererPtr,
    color: &[f32; 3],
    intensity: f32,
) {
    let renderer = &mut *(renderer as *mut Renderer);
    renderer.set_ambient(*color, intensity);
}

#[no_mangle]
pub unsafe extern "C" fn helio_renderer_set_debug_mode(
    renderer: HelioRendererPtr,
    mode: u32,
) {
    let renderer = &mut *(renderer as *mut Renderer);
    renderer.set_debug_mode(mode);
}

#[no_mangle]
pub unsafe extern "C" fn helio_renderer_set_editor_mode(
    renderer: HelioRendererPtr,
    enabled: bool,
) {
    let renderer = &mut *(renderer as *mut Renderer);
    renderer.set_editor_mode(enabled);
}

#[no_mangle]
pub unsafe extern "C" fn helio_renderer_set_shadow_quality(
    renderer: HelioRendererPtr,
    quality: u32,
) {
    let renderer = &mut *(renderer as *mut Renderer);
    let q = match quality {
        0 => libhelio::ShadowQuality::Low,
        1 => libhelio::ShadowQuality::Medium,
        2 => libhelio::ShadowQuality::High,
        _ => libhelio::ShadowQuality::Ultra,
    };
    renderer.set_shadow_quality(q);
}

#[no_mangle]
pub unsafe extern "C" fn helio_renderer_output_size(
    renderer: HelioRendererPtr,
    out_width: &mut u32,
    out_height: &mut u32,
) {
    let renderer = &*(renderer as *const Renderer);
    *out_width = renderer.output_width();
    *out_height = renderer.output_height();
}

// ── Debug draw ─────────────────────────────────────────────────────────────────

#[no_mangle]
pub unsafe extern "C" fn helio_renderer_debug_line(
    renderer: HelioRendererPtr,
    from: &[f32; 3],
    to: &[f32; 3],
    color: &[f32; 4],
) {
    let renderer = &mut *(renderer as *mut Renderer);
    renderer.debug_batch(|batch| {
        batch.line(*from, *to, *color);
    });
}

#[no_mangle]
pub unsafe extern "C" fn helio_renderer_debug_sphere(
    renderer: HelioRendererPtr,
    center: &[f32; 3],
    radius: f32,
    color: &[f32; 4],
    segments: u32,
) {
    let renderer = &mut *(renderer as *mut Renderer);
    renderer.debug_batch(|batch| {
        batch.sphere(*center, radius, *color, segments);
    });
}
