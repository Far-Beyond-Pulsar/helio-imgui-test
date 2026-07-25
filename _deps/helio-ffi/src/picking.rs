use helio::{ScenePicker, Scene, PackedVertex};

use crate::types::*;

#[no_mangle]
pub unsafe extern "C" fn helio_picker_new() -> HelioPickerPtr {
    Box::into_raw(Box::new(ScenePicker::new())) as HelioPickerPtr
}

#[no_mangle]
pub unsafe extern "C" fn helio_picker_destroy(picker: HelioPickerPtr) {
    if !picker.is_null() {
        drop(Box::from_raw(picker as *mut ScenePicker));
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_picker_register_mesh(
    picker: HelioPickerPtr,
    mesh_id: HelioHandle,
    vertex_data: *const u8,
    vertex_count: u32,
    index_data: *const u8,
    index_count: u32,
) {
    let picker = &mut *(picker as *mut ScenePicker);
    let vertices = unsafe {
        std::slice::from_raw_parts(vertex_data as *const PackedVertex, vertex_count as usize)
    };
    let indices = unsafe {
        std::slice::from_raw_parts(index_data as *const u32, index_count as usize)
    };
    let upload = helio::MeshUpload {
        vertices: vertices.to_vec(),
        indices: indices.to_vec(),
    };
    picker.register_mesh(mesh_id.into(), &upload);
}

#[no_mangle]
pub unsafe extern "C" fn helio_picker_rebuild_instances(
    picker: HelioPickerPtr,
    scene: HelioScenePtr,
) {
    let picker = &mut *(picker as *mut ScenePicker);
    let scene = &*(scene as *const Scene);
    picker.rebuild_instances(scene);
}

#[no_mangle]
pub unsafe extern "C" fn helio_picker_cast_ray(
    picker: HelioPickerPtr,
    scene: HelioScenePtr,
    origin: &[f32; 3],
    dir: &[f32; 3],
    out_hit: &mut HelioPickHit,
) -> bool {
    let picker = &*(picker as *const ScenePicker);
    let scene = &*(scene as *const Scene);
    if let Some(hit) = picker.cast_ray(scene, (*origin).into(), (*dir).into()) {
        *out_hit = HelioPickHit {
            actor_id: hit.actor_id.into(),
            t: hit.t,
            position: hit.position.into(),
            normal: hit.normal.into(),
            user_tag: hit.user_tag,
        };
        true
    } else {
        false
    }
}

/// Convenience: ray from two points.
#[no_mangle]
pub unsafe extern "C" fn helio_picker_cast_ray_from_to(
    picker: HelioPickerPtr,
    scene: HelioScenePtr,
    from: &[f32; 3],
    to: &[f32; 3],
    out_hit: &mut HelioPickHit,
) -> bool {
    let origin: glam::Vec3 = (*from).into();
    let target: glam::Vec3 = (*to).into();
    let dir = target - origin;
    let len = dir.length();
    if len < 1e-8 {
        return false;
    }
    let dir = dir / len;
    helio_picker_cast_ray(picker, scene, from, &dir.to_array(), out_hit)
}
