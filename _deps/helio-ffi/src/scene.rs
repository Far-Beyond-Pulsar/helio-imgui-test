use std::sync::Arc;
use std::ffi::CStr;

use helio::{
    GroupId, GroupMask, MeshUpload, Movability, ObjectDescriptor, ReflectionCaptureDescriptor,
    Scene, TextureSamplerDesc, TextureUpload, VirtualMeshUpload, VirtualObjectDescriptor,
    VoxelVolumeDescriptor, WaterVolumeDescriptor, WaterHitboxDescriptor,
};
use helio_core::GpuMaterial;
use libhelio::GpuLight;

use crate::types::*;
use crate::camera::camera_from_desc;

// ── Scene construction ─────────────────────────────────────────────────────────

#[no_mangle]
pub unsafe extern "C" fn helio_scene_new(
    device: HelioDevicePtr,
    queue: HelioQueuePtr,
) -> HelioScenePtr {
    let device = Arc::from_raw(device as *const wgpu::Device);
    let queue = Arc::from_raw(queue as *const wgpu::Queue);
    let scene = Box::new(Scene::new(device.clone(), queue.clone()));
    std::mem::forget(device);
    std::mem::forget(queue);
    Box::into_raw(scene) as HelioScenePtr
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_destroy(scene: HelioScenePtr) {
    if !scene.is_null() {
        drop(Box::from_raw(scene as *mut Scene));
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_flush(scene: HelioScenePtr) {
    let scene = &mut *(scene as *mut Scene);
    scene.flush();
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_advance_frame(scene: HelioScenePtr) {
    let scene = &mut *(scene as *mut Scene);
    scene.advance_frame();
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_set_render_size(
    scene: HelioScenePtr,
    width: u32,
    height: u32,
) {
    let scene = &mut *(scene as *mut Scene);
    scene.set_render_size(width, height);
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_clear(scene: HelioScenePtr) {
    let scene = &mut *(scene as *mut Scene);
    scene.clear();
}

// ── Camera ─────────────────────────────────────────────────────────────────────

#[no_mangle]
pub unsafe extern "C" fn helio_scene_update_camera(
    scene: HelioScenePtr,
    camera: &HelioCameraDesc,
) {
    let scene = &mut *(scene as *mut Scene);
    scene.update_camera(camera_from_desc(camera));
}

// ── Groups ─────────────────────────────────────────────────────────────────────

#[no_mangle]
pub unsafe extern "C" fn helio_scene_hide_group(scene: HelioScenePtr, group_index: u8) {
    let scene = &mut *(scene as *mut Scene);
    scene.hide_group(GroupId::new(group_index));
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_show_group(scene: HelioScenePtr, group_index: u8) {
    let scene = &mut *(scene as *mut Scene);
    scene.show_group(GroupId::new(group_index));
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_is_group_hidden(
    scene: HelioScenePtr,
    group_index: u8,
) -> bool {
    let scene = &mut *(scene as *mut Scene);
    scene.is_group_hidden(GroupId::new(group_index))
}

// ── Meshes ─────────────────────────────────────────────────────────────────────

fn packed_vertex_slice<'a>(upload: &HelioMeshUpload) -> &'a [helio::PackedVertex] {
    unsafe {
        std::slice::from_raw_parts(
            upload.vertices as *const helio::PackedVertex,
            upload.vertex_count as usize,
        )
    }
}

fn index_slice<'a>(upload: &HelioMeshUpload) -> &'a [u32] {
    unsafe {
        std::slice::from_raw_parts(upload.indices, upload.index_count as usize)
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_mesh(
    scene: HelioScenePtr,
    upload: &HelioMeshUpload,
) -> HelioHandle {
    let scene = &mut *(scene as *mut Scene);
    let id = scene.insert_mesh(MeshUpload {
        vertices: packed_vertex_slice(upload).to_vec(),
        indices: index_slice(upload).to_vec(),
    });
    id.into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_dynamic_mesh(
    scene: HelioScenePtr,
    upload: &HelioMeshUpload,
) -> HelioHandle {
    let scene = &mut *(scene as *mut Scene);
    let id = scene.insert_dynamic_mesh(MeshUpload {
        vertices: packed_vertex_slice(upload).to_vec(),
        indices: index_slice(upload).to_vec(),
    });
    id.into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_remove_mesh(
    scene: HelioScenePtr,
    mesh: HelioHandle,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.remove_mesh(mesh.into()).into()
}

// ── Materials ──────────────────────────────────────────────────────────────────

fn build_gpu_material(m: &HelioGpuMaterial) -> GpuMaterial {
    GpuMaterial {
        base_color: m.base_color,
        emissive: m.emissive,
        roughness_metallic: m.roughness_metallic,
        tex_base_color: m.tex_base_color,
        tex_normal: m.tex_normal,
        tex_roughness: m.tex_roughness,
        tex_emissive: m.tex_emissive,
        tex_occlusion: m.tex_occlusion,
        workflow: m.workflow,
        flags: m.flags,
        material_class: m.material_class,
        class_params: m.class_params,
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_material(
    scene: HelioScenePtr,
    material: &HelioGpuMaterial,
) -> HelioHandle {
    let scene = &mut *(scene as *mut Scene);
    let id = scene.insert_material(build_gpu_material(material));
    id.into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_remove_material(
    scene: HelioScenePtr,
    material: HelioHandle,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.remove_material(material.into()).into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_update_material(
    scene: HelioScenePtr,
    material: HelioHandle,
    data: &HelioGpuMaterial,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.update_material(material.into(), build_gpu_material(data)).into()
}

// ── Textures ───────────────────────────────────────────────────────────────────

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

fn wgpu_address_mode(m: u32) -> wgpu::AddressMode {
    match m {
        0 => wgpu::AddressMode::Repeat,
        1 => wgpu::AddressMode::MirrorRepeat,
        2 => wgpu::AddressMode::ClampToEdge,
        3 => wgpu::AddressMode::ClampToBorder,
        _ => wgpu::AddressMode::Repeat,
    }
}

fn wgpu_filter_mode(m: u32) -> wgpu::FilterMode {
    match m {
        0 => wgpu::FilterMode::Nearest,
        1 => wgpu::FilterMode::Linear,
        _ => wgpu::FilterMode::Linear,
    }
}

fn wgpu_mipmap_filter_mode(m: u32) -> wgpu::MipmapFilterMode {
    match m {
        0 => wgpu::MipmapFilterMode::Nearest,
        1 => wgpu::MipmapFilterMode::Linear,
        _ => wgpu::MipmapFilterMode::Linear,
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_texture(
    scene: HelioScenePtr,
    upload: &HelioTextureUpload,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    let data = std::slice::from_raw_parts(upload.data, upload.data_len).to_vec();
    let label = if upload.label.is_null() {
        None
    } else {
        Some(CStr::from_ptr(upload.label).to_string_lossy().into_owned())
    };
    let tex = TextureUpload {
        label,
        width: upload.width,
        height: upload.height,
        format: wgpu_texture_format(upload.format),
        data,
        sampler: TextureSamplerDesc {
            address_mode_u: wgpu_address_mode(upload.address_mode_u),
            address_mode_v: wgpu_address_mode(upload.address_mode_v),
            address_mode_w: wgpu_address_mode(upload.address_mode_w),
            mag_filter: wgpu_filter_mode(upload.mag_filter),
            min_filter: wgpu_filter_mode(upload.min_filter),
            mipmap_filter: wgpu_mipmap_filter_mode(upload.mipmap_filter),
        },
    };
    scene.insert_texture(tex).into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_remove_texture(
    scene: HelioScenePtr,
    texture: HelioHandle,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.remove_texture(texture.into()).into()
}

// ── Lights ─────────────────────────────────────────────────────────────────────

fn build_gpu_light(l: &HelioGpuLight) -> GpuLight {
    GpuLight {
        position_range: l.position_range,
        direction_outer: l.direction_outer,
        color_intensity: l.color_intensity,
        shadow_index: l.shadow_index,
        light_type: l.light_type,
        inner_angle: l.inner_angle,
        _pad: l._pad,
        god_rays_enabled: l.god_rays_enabled,
        god_rays_density: l.god_rays_density,
        god_rays_weight: l.god_rays_weight,
        god_rays_decay: l.god_rays_decay,
        god_rays_exposure: l.god_rays_exposure,
        _pad2: l._pad2,
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_light(
    scene: HelioScenePtr,
    light: &HelioGpuLight,
) -> HelioHandle {
    let scene = &mut *(scene as *mut Scene);
    let id = scene.insert_light(build_gpu_light(light));
    id.into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_light_with_tag(
    scene: HelioScenePtr,
    light: &HelioGpuLight,
    user_tag: u64,
    movability: u32,
) -> HelioHandle {
    let scene = &mut *(scene as *mut Scene);
    let mov = match movability {
        0 => None,
        1 => Some(Movability::Stationary),
        _ => Some(Movability::Movable),
    };
    let id = scene.insert_light_with_movability(build_gpu_light(light), mov, user_tag);
    id.into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_remove_light(
    scene: HelioScenePtr,
    light: HelioHandle,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.remove_light(light.into()).into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_update_light(
    scene: HelioScenePtr,
    light: HelioHandle,
    data: &HelioGpuLight,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.update_light(light.into(), build_gpu_light(data)).into()
}

// ── Objects ────────────────────────────────────────────────────────────────────

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_object(
    scene: HelioScenePtr,
    desc: &HelioObjectDescriptor,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    let movability = match desc.movability {
        0 => None,
        1 => Some(Movability::Stationary),
        _ => Some(Movability::Movable),
    };
    let od = ObjectDescriptor {
        mesh: desc.mesh.into(),
        material: desc.material.into(),
        transform: desc.transform.into(),
        bounds: desc.bounds,
        flags: desc.flags,
        groups: GroupMask(desc.groups),
        movability,
        user_tag: desc.user_tag,
    };
    scene.insert_object(od).into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_remove_object(
    scene: HelioScenePtr,
    object: HelioHandle,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.remove_object(object.into()).into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_update_object_transform(
    scene: HelioScenePtr,
    object: HelioHandle,
    transform: &HelioMat4,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.update_object_transform(object.into(), (*transform).into()).into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_get_object_transform(
    scene: HelioScenePtr,
    object: HelioHandle,
    out_transform: &mut HelioMat4,
) -> bool {
    let scene = &mut *(scene as *mut Scene);
    match scene.get_object_transform(object.into()) {
        Ok(t) => {
            *out_transform = t.into();
            true
        }
        Err(_) => false,
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_update_object_material(
    scene: HelioScenePtr,
    object: HelioHandle,
    material: HelioHandle,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.update_object_material(object.into(), material.into()).into()
}

// ── Material class (via scene) ─────────────────────────────────────────────────

#[no_mangle]
pub unsafe extern "C" fn helio_scene_set_material_class(
    scene: HelioScenePtr,
    material: HelioHandle,
    material_class: u32,
    graph_hash: u64,
    feature_flags: u32,
    use_flags: bool,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene
        .set_material_class(
            material.into(),
            material_class,
            graph_hash,
            if use_flags { Some(feature_flags) } else { None },
        )
        .into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_update_material_class_params(
    scene: HelioScenePtr,
    material: HelioHandle,
    params: &[f32; 4],
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.update_material_class_params(material.into(), *params).into()
}

// ── Water volumes ──────────────────────────────────────────────────────────────

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_water_volume(
    scene: HelioScenePtr,
    desc: &HelioWaterVolumeDesc,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    let wv = WaterVolumeDescriptor {
        bounds_min: desc.bounds_min,
        bounds_max: desc.bounds_max,
        surface_height: desc.surface_height,
        wave_amplitude: desc.wave_amplitude,
        wave_frequency: desc.wave_frequency,
        wave_speed: desc.wave_speed,
        wave_direction: desc.wave_direction,
        wave_steepness: desc.wave_steepness,
        water_color: desc.water_color,
        extinction: desc.extinction,
        foam_threshold: desc.foam_threshold,
        foam_amount: desc.foam_amount,
        reflection_strength: desc.reflection_strength,
        refraction_strength: desc.refraction_strength,
        fresnel_power: desc.fresnel_power,
        caustics_enabled: desc.caustics_enabled,
        caustics_intensity: desc.caustics_intensity,
        caustics_scale: desc.caustics_scale,
        caustics_speed: desc.caustics_speed,
        fog_density: desc.fog_density,
        god_rays_intensity: desc.god_rays_intensity,
        ssr_enabled: desc.ssr_enabled,
        ssr_steps: desc.ssr_steps,
        ssr_step_size: desc.ssr_step_size,
        ssr_thickness: desc.ssr_thickness,
        ior: desc.ior,
        fresnel_min: desc.fresnel_min,
        density: desc.density,
        shadow_rim: desc.shadow_rim,
        shadow_hitbox: desc.shadow_hitbox,
        shadow_ao: desc.shadow_ao,
        sun_direction: desc.sun_direction,
        wave_spring: desc.wave_spring,
        wave_damping: desc.wave_damping,
        wind_direction: desc.wind_direction,
        wind_strength: desc.wind_strength,
        wave_scale: desc.wave_scale,
    };
    scene.insert_water_volume(wv).into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_remove_water_volume(
    scene: HelioScenePtr,
    volume: HelioHandle,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.remove_water_volume(volume.into()).into()
}

// ── Water hitboxes ─────────────────────────────────────────────────────────────

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_water_hitbox(
    scene: HelioScenePtr,
    desc: &HelioWaterHitboxDesc,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    let hb = WaterHitboxDescriptor {
        old_min: desc.old_min,
        old_max: desc.old_max,
        new_min: desc.new_min,
        new_max: desc.new_max,
        edge_softness: desc.edge_softness,
        strength: desc.strength,
    };
    scene.insert_water_hitbox(hb).into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_remove_water_hitbox(
    scene: HelioScenePtr,
    hitbox: HelioHandle,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.remove_water_hitbox(hitbox.into()).into()
}

// ── Decals ─────────────────────────────────────────────────────────────────────

fn build_gpu_decal(d: &HelioGpuDecal) -> libhelio::GpuDecal {
    libhelio::GpuDecal {
        transform: d.transform,
        color: d.color,
        albedo_texture_index: d.albedo_texture_index,
        normal_texture_index: d.normal_texture_index,
        roughness_texture_index: d.roughness_texture_index,
        metalness_texture_index: d.metalness_texture_index,
        blend_mode: d.blend_mode,
        decal_type: d.decal_type,
        fade_time: d.fade_time,
        fade_start_delay: d.fade_start_delay,
        age: d.age,
        normal_adapt: d.normal_adapt,
        _pad0: d._pad0,
        _pad1: d._pad1,
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_decal(
    scene: HelioScenePtr,
    decal: &HelioGpuDecal,
) -> HelioHandle {
    let scene = &mut *(scene as *mut Scene);
    let id = scene.insert_decal(build_gpu_decal(decal));
    id.into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_remove_decal(
    scene: HelioScenePtr,
    decal: HelioHandle,
) -> bool {
    let scene = &mut *(scene as *mut Scene);
    scene.remove_decal(decal.into())
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_update_decal(
    scene: HelioScenePtr,
    decal: HelioHandle,
    data: &HelioGpuDecal,
) -> bool {
    let scene = &mut *(scene as *mut Scene);
    scene.update_decal(decal.into(), build_gpu_decal(data))
}

// ── Reflection captures ────────────────────────────────────────────────────────

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_reflection_capture(
    scene: HelioScenePtr,
    desc: &HelioReflectionCaptureDesc,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    use libhelio::{ReflectionCaptureShape, ReflectionCaptureMobility};
    let rc = ReflectionCaptureDescriptor {
        shape: if desc.shape == 0 {
            ReflectionCaptureShape::Sphere
        } else {
            ReflectionCaptureShape::Box
        },
        mobility: if desc.mobility == 0 {
            ReflectionCaptureMobility::Static
        } else {
            ReflectionCaptureMobility::Dynamic
        },
        transform: desc.transform.into(),
        influence_radius: desc.influence_radius,
        extents: desc.extents,
        transition_distance: desc.transition_distance,
        brightness: desc.brightness,
    };
    scene.insert_reflection_capture(rc).into()
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_remove_reflection_capture(
    scene: HelioScenePtr,
    capture: HelioHandle,
) -> bool {
    let scene = &mut *(scene as *mut Scene);
    scene.remove_reflection_capture(capture.into())
}

// ── Voxel volumes ──────────────────────────────────────────────────────────────

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_voxel_volume(
    scene: HelioScenePtr,
    desc: &HelioVoxelVolumeDescriptor,
) -> HelioHandle {
    let scene = &mut *(scene as *mut Scene);
    let vv = VoxelVolumeDescriptor {
        voxel_size: desc.voxel_size,
        root_extent: desc.root_extent,
        local_to_world: desc.local_to_world.into(),
        movability: if desc.movability == 2 {
            Some(Movability::Movable)
        } else {
            None
        },
        mode: None,
        material_palette: Vec::new(),
    };
    match scene.insert_voxel_volume(vv) {
        Ok(id) => id.into(),
        Err(_) => HelioHandle::null(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_remove_voxel_volume(
    scene: HelioScenePtr,
    volume: HelioHandle,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    scene.remove_voxel_volume(volume.into()).into()
}

// ── Virtual geometry ───────────────────────────────────────────────────────────

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_virtual_mesh(
    scene: HelioScenePtr,
    upload: &HelioMeshUpload,
) -> HelioHandle {
    let scene = &mut *(scene as *mut Scene);
    let vm = VirtualMeshUpload {
        vertices: packed_vertex_slice(upload).to_vec(),
        indices: index_slice(upload).to_vec(),
    };
    let id = scene.insert_virtual_mesh(vm);
    HelioHandle {
        slot: id.0,
        generation: 0,
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_scene_insert_virtual_object(
    scene: HelioScenePtr,
    desc: &HelioVirtualObjectDescriptor,
) -> HelioResult {
    let scene = &mut *(scene as *mut Scene);
    let vd = VirtualObjectDescriptor {
        virtual_mesh: helio::VirtualMeshId(desc.virtual_mesh.slot),
        material_id: desc.material_id,
        transform: desc.transform.into(),
        bounds: desc.bounds,
        flags: desc.flags,
        groups: GroupMask(desc.groups),
        movability: if desc.movability == 2 {
            Some(Movability::Movable)
        } else {
            None
        },
    };
    scene.insert_virtual_object(vd).into()
}
