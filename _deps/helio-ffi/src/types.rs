use std::ffi::{CStr, CString};
use std::os::raw::c_char;

/// Opaque handle types — Rust side stores a Box<...> and passes a raw pointer.
pub type HelioScenePtr = *mut std::ffi::c_void;
pub type HelioRendererPtr = *mut std::ffi::c_void;
pub type HelioPickerPtr = *mut std::ffi::c_void;
pub type HelioEditorPtr = *mut std::ffi::c_void;
pub type HelioDevicePtr = *mut std::ffi::c_void;
pub type HelioQueuePtr = *mut std::ffi::c_void;
pub type HelioTextureViewPtr = *mut std::ffi::c_void;
pub type HelioBufferPtr = *mut std::ffi::c_void;
pub type HelioMeshUploadPtr = *mut std::ffi::c_void;

// ── Result type ────────────────────────────────────────────────────────────────

#[repr(C)]
pub struct HelioResult {
    pub success: bool,
    pub error_message: *mut c_char,
}

impl HelioResult {
    pub fn ok() -> Self {
        Self {
            success: true,
            error_message: std::ptr::null_mut(),
        }
    }

    pub fn err(msg: &str) -> Self {
        let cmsg = CString::new(msg).unwrap_or_default();
        Self {
            success: false,
            error_message: cmsg.into_raw(),
        }
    }
}

impl<T, E: std::fmt::Display> From<Result<T, E>> for HelioResult {
    fn from(r: Result<T, E>) -> Self {
        match r {
            Ok(_) => Self::ok(),
            Err(e) => Self::err(&e.to_string()),
        }
    }
}

/// Free an error string returned by any function.
#[no_mangle]
pub unsafe extern "C" fn helio_free_error_string(s: *mut c_char) {
    if !s.is_null() {
        drop(CString::from_raw(s));
    }
}

// ── Math types ─────────────────────────────────────────────────────────────────

#[repr(C)]
#[derive(Debug, Clone, Copy, Default)]
pub struct HelioVec3 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

impl From<HelioVec3> for glam::Vec3 {
    fn from(v: HelioVec3) -> Self {
        glam::Vec3::new(v.x, v.y, v.z)
    }
}

impl From<glam::Vec3> for HelioVec3 {
    fn from(v: glam::Vec3) -> Self {
        Self {
            x: v.x,
            y: v.y,
            z: v.z,
        }
    }
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct HelioMat4 {
    pub data: [[f32; 4]; 4],
}

impl From<HelioMat4> for glam::Mat4 {
    fn from(m: HelioMat4) -> Self {
        glam::Mat4::from_cols_array_2d(&m.data)
    }
}

impl From<glam::Mat4> for HelioMat4 {
    fn from(m: glam::Mat4) -> Self {
        Self {
            data: m.to_cols_array_2d(),
        }
    }
}

// ── Handle types ───────────────────────────────────────────────────────────────

#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct HelioHandle {
    pub slot: u32,
    pub generation: u32,
}

impl HelioHandle {
    pub const fn null() -> Self {
        Self {
            slot: u32::MAX,
            generation: 0,
        }
    }

    pub fn is_null(&self) -> bool {
        self.slot == u32::MAX
    }
}

impl Default for HelioHandle {
    fn default() -> Self {
        Self::null()
    }
}

// ── SceneActorId conversion ────────────────────────────────────────────────────

/// Actor type discriminator (must match `HelioSceneActorType`)
#[repr(u32)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum HelioSceneActorType {
    None = 0,
    Decal = 1,
    Mesh = 2,
    Light = 3,
    ReflectionCapture = 4,
    VirtualMesh = 5,
    VirtualObject = 6,
    Object = 7,
    SectionedObject = 8,
    WaterVolume = 9,
    WaterHitbox = 10,
    PostProcessVolume = 11,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct HelioSceneActorId {
    pub actor_type: HelioSceneActorType,
    pub handle: HelioHandle,
}

impl HelioSceneActorId {
    pub fn none() -> Self {
        Self {
            actor_type: HelioSceneActorType::None,
            handle: HelioHandle::null(),
        }
    }
}

impl Default for HelioSceneActorId {
    fn default() -> Self {
        Self::none()
    }
}

impl From<HelioSceneActorId> for helio::SceneActorId {
    fn from(id: HelioSceneActorId) -> Self {
        use helio::SceneActorId as S;
        match id.actor_type {
            HelioSceneActorType::None => S::None,
            HelioSceneActorType::Decal => S::Decal(id.handle.into()),
            HelioSceneActorType::Mesh => S::Mesh(id.handle.into()),
            HelioSceneActorType::Light => S::Light(id.handle.into()),
            HelioSceneActorType::ReflectionCapture => S::ReflectionCapture(id.handle.into()),
            HelioSceneActorType::VirtualMesh => S::VirtualMesh(helio::VirtualMeshId(id.handle.slot)),
            HelioSceneActorType::VirtualObject => S::VirtualObject(id.handle.into()),
            HelioSceneActorType::Object => S::Object(id.handle.into()),
            HelioSceneActorType::SectionedObject => S::SectionedObject(id.handle.into()),
            HelioSceneActorType::WaterVolume => S::WaterVolume(id.handle.into()),
            HelioSceneActorType::WaterHitbox => S::WaterHitbox(id.handle.into()),
            HelioSceneActorType::PostProcessVolume => S::PostProcessVolume(id.handle.into()),
        }
    }
}

impl From<helio::SceneActorId> for HelioSceneActorId {
    fn from(id: helio::SceneActorId) -> Self {
        use helio::SceneActorId as S;
        let (ty, handle) = match id {
            S::None => (HelioSceneActorType::None, HelioHandle::null()),
            S::Decal(h) => (HelioSceneActorType::Decal, h.into()),
            S::Mesh(h) => (HelioSceneActorType::Mesh, h.into()),
            S::Light(h) => (HelioSceneActorType::Light, h.into()),
            S::ReflectionCapture(h) => (HelioSceneActorType::ReflectionCapture, h.into()),
            S::VirtualMesh(h) => (HelioSceneActorType::VirtualMesh, HelioHandle { slot: h.0, generation: 0 }),
            S::VirtualObject(h) => (HelioSceneActorType::VirtualObject, h.into()),
            S::Object(h) => (HelioSceneActorType::Object, h.into()),
            S::SectionedObject(h) => (HelioSceneActorType::SectionedObject, h.into()),
            S::WaterVolume(h) => (HelioSceneActorType::WaterVolume, h.into()),
            S::WaterHitbox(h) => (HelioSceneActorType::WaterHitbox, h.into()),
            S::PostProcessVolume(h) => (HelioSceneActorType::PostProcessVolume, h.into()),
        };
        Self {
            actor_type: ty,
            handle,
        }
    }
}

// ── Pick hit ───────────────────────────────────────────────────────────────────

#[repr(C)]
pub struct HelioPickHit {
    pub actor_id: HelioSceneActorId,
    pub t: f32,
    pub position: HelioVec3,
    pub normal: HelioVec3,
    pub user_tag: u64,
}

// ── Descriptor types ───────────────────────────────────────────────────────────

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct HelioObjectDescriptor {
    pub mesh: HelioHandle,
    pub material: HelioHandle,
    pub transform: HelioMat4,
    pub bounds: [f32; 4],
    pub flags: u32,
    pub groups: u64,
    pub movability: u32,
    pub user_tag: u64,
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct HelioCameraDesc {
    pub view: HelioMat4,
    pub proj: HelioMat4,
    pub position: HelioVec3,
    pub near: f32,
    pub far: f32,
    pub jitter: [f32; 2],
}

/// Matches `libhelio::GpuLight` layout
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct HelioGpuLight {
    pub position_range: [f32; 4],
    pub direction_outer: [f32; 4],
    pub color_intensity: [f32; 4],
    pub shadow_index: u32,
    pub light_type: u32,
    pub inner_angle: f32,
    pub _pad: u32,
    pub god_rays_enabled: u32,
    pub god_rays_density: f32,
    pub god_rays_weight: f32,
    pub god_rays_decay: f32,
    pub god_rays_exposure: f32,
    pub _pad2: [u32; 3],
}

/// Matches `libhelio::GpuMaterial` layout
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct HelioGpuMaterial {
    pub base_color: [f32; 4],
    pub emissive: [f32; 4],
    pub roughness_metallic: [f32; 4],
    pub tex_base_color: u32,
    pub tex_normal: u32,
    pub tex_roughness: u32,
    pub tex_emissive: u32,
    pub tex_occlusion: u32,
    pub workflow: u32,
    pub flags: u32,
    pub material_class: u32,
    pub class_params: [f32; 4],
}

/// Flat vertex/index data for mesh uploads.
#[repr(C)]
#[derive(Debug, Clone)]
pub struct HelioMeshUpload {
    pub vertices: *const HelioPackedVertex,
    pub vertex_count: u32,
    pub indices: *const u32,
    pub index_count: u32,
}

/// Layout-compatible with `helio::PackedVertex`
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct HelioPackedVertex {
    pub position: [f32; 3],
    pub bitangent_sign: f32,
    pub tex_coords0: [f32; 2],
    pub tex_coords1: [f32; 2],
    pub normal: u32,
    pub tangent: u32,
}

/// Matches `libhelio::GpuDecal` layout
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct HelioGpuDecal {
    pub transform: [f32; 16],
    pub color: [f32; 4],
    pub albedo_texture_index: u32,
    pub normal_texture_index: u32,
    pub roughness_texture_index: u32,
    pub metalness_texture_index: u32,
    pub blend_mode: u32,
    pub decal_type: u32,
    pub fade_time: f32,
    pub fade_start_delay: f32,
    pub age: f32,
    pub normal_adapt: u32,
    pub _pad0: f32,
    pub _pad1: f32,
}

/// Matches `libhelio::GpuWaterVolume` layout
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct HelioGpuWaterVolume {
    pub bounds_min: [f32; 4],
    pub bounds_max: [f32; 4],
    pub wave_params: [f32; 4],
    pub wave_direction: [f32; 4],
    pub water_color: [f32; 4],
    pub extinction: [f32; 4],
    pub reflection_refraction: [f32; 4],
    pub caustics_params: [f32; 4],
    pub fog_params: [f32; 4],
    pub sim_params: [f32; 4],
    pub shadow_params: [f32; 4],
    pub sun_direction: [f32; 4],
    pub ssr_params: [f32; 4],
    pub sim_dynamics: [f32; 4],
    pub wind_params: [f32; 4],
    pub _pad6: [f32; 4],
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct HelioWaterVolumeDesc {
    pub bounds_min: [f32; 3],
    pub bounds_max: [f32; 3],
    pub surface_height: f32,
    pub wave_amplitude: f32,
    pub wave_frequency: f32,
    pub wave_speed: f32,
    pub wave_direction: [f32; 2],
    pub wave_steepness: f32,
    pub water_color: [f32; 3],
    pub extinction: [f32; 3],
    pub foam_threshold: f32,
    pub foam_amount: f32,
    pub reflection_strength: f32,
    pub refraction_strength: f32,
    pub fresnel_power: f32,
    pub caustics_enabled: bool,
    pub caustics_intensity: f32,
    pub caustics_scale: f32,
    pub caustics_speed: f32,
    pub fog_density: f32,
    pub god_rays_intensity: f32,
    pub ssr_enabled: bool,
    pub ssr_steps: u32,
    pub ssr_step_size: f32,
    pub ssr_thickness: f32,
    pub ior: f32,
    pub fresnel_min: f32,
    pub density: f32,
    pub shadow_rim: f32,
    pub shadow_hitbox: f32,
    pub shadow_ao: f32,
    pub sun_direction: [f32; 3],
    pub wave_spring: f32,
    pub wave_damping: f32,
    pub wind_direction: [f32; 2],
    pub wind_strength: f32,
    pub wave_scale: f32,
}

/// Matches `libhelio::GpuWaterHitbox` layout
#[repr(C)]
pub struct HelioGpuWaterHitbox {
    pub old_min: [f32; 4],
    pub old_max: [f32; 4],
    pub new_min: [f32; 4],
    pub new_max: [f32; 4],
    pub params: [f32; 4],
}

#[repr(C)]
pub struct HelioWaterHitboxDesc {
    pub old_min: [f32; 3],
    pub old_max: [f32; 3],
    pub new_min: [f32; 3],
    pub new_max: [f32; 3],
    pub edge_softness: f32,
    pub strength: f32,
}

#[repr(C)]
pub struct HelioGpuPostProcessVolume {
    pub bounds_min: [f32; 4],
    pub bounds_max: [f32; 4],
    pub settings: [f32; 12],
    pub blend_priority: f32,
    pub blend_falloff: f32,
    pub _pad: [f32; 2],
}

#[repr(C)]
pub struct HelioPostProcessVolumeDesc {
    pub bounds_min: [f32; 3],
    pub bounds_max: [f32; 3],
    pub blend_priority: f32,
    pub blend_falloff: f32,
    pub exposure: f32,
    pub bloom_intensity: f32,
    pub bloom_threshold: f32,
    pub tonemap_mode: u32,
}

/// Gizmo mode constants
#[repr(u32)]
pub enum HelioGizmoMode {
    Translate = 0,
    Rotate = 1,
    Scale = 2,
}

/// Gizmo axis constants
#[repr(u32)]
pub enum HelioGizmoAxis {
    X = 0,
    Y = 1,
    Z = 2,
    W = 3,
    None = 4,
}

#[repr(C)]
pub struct HelioReflectionCaptureDesc {
    pub shape: u32,
    pub mobility: u32,
    pub transform: HelioMat4,
    pub influence_radius: f32,
    pub extents: [f32; 3],
    pub transition_distance: f32,
    pub brightness: f32,
}

#[repr(C)]
pub struct HelioGpuReflectionCapture {
    pub position: [f32; 4],
    pub shape_data: [f32; 4],
    pub brightness_blend: [f32; 4],
}

#[repr(C)]
pub struct HelioVoxelVolumeDescriptor {
    pub voxel_size: f32,
    pub root_extent: f32,
    pub local_to_world: HelioMat4,
    pub movability: u32,
    pub mode: u32,
}

/// Renderer config
#[repr(C)]
pub struct HelioRendererConfig {
    pub width: u32,
    pub height: u32,
    pub surface_format: u32,
    pub gi_rc_radius: f32,
    pub gi_fade_margin: f32,
    pub shadow_quality: u32,
    pub debug_mode: u32,
    pub render_scale: f32,
    pub perf_overlay_mode: u32,
    pub shadow_atlas_size: u32,
    pub shadow_face_capacity: u32,
}

impl Default for HelioRendererConfig {
    fn default() -> Self {
        Self {
            width: 1920,
            height: 1080,
            surface_format: 0,
            gi_rc_radius: 80.0,
            gi_fade_margin: 20.0,
            shadow_quality: 1,
            debug_mode: 0,
            render_scale: 1.0,
            perf_overlay_mode: 0,
            shadow_atlas_size: 1024,
            shadow_face_capacity: 32,
        }
    }
}

#[repr(C)]
pub struct HelioTextureUpload {
    pub label: *const c_char,
    pub width: u32,
    pub height: u32,
    pub format: u32,
    pub data: *const u8,
    pub data_len: usize,
    pub address_mode_u: u32,
    pub address_mode_v: u32,
    pub address_mode_w: u32,
    pub mag_filter: u32,
    pub min_filter: u32,
    pub mipmap_filter: u32,
}

#[repr(C)]
pub struct HelioGpuMeshletEntry {
    pub center: [f32; 3],
    pub radius: f32,
    pub cone_apex: [f32; 3],
    pub cone_cutoff: f32,
    pub cone_axis: [f32; 3],
    pub lod_error: f32,
    pub first_index: u32,
    pub index_count: u32,
    pub vertex_offset: i32,
    pub instance_index: u32,
}

#[repr(C)]
pub struct HelioVirtualObjectDescriptor {
    pub virtual_mesh: HelioHandle,
    pub material_id: u32,
    pub transform: HelioMat4,
    pub bounds: [f32; 4],
    pub flags: u32,
    pub groups: u64,
    pub movability: u32,
}
