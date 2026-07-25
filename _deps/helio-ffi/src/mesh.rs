use helio::{MeshUpload, PackedVertex, SectionedMeshUpload};

use crate::types::*;

/// Build a `HelioMeshUpload` from raw vertex/index data, converting to the
/// opqaue `HelioMeshUploadPtr` (a `Box<MeshUpload>`).
/// Returns the opaque pointer; caller must pass it to `helio_mesh_upload_free`.
#[no_mangle]
pub unsafe extern "C" fn helio_mesh_upload_create(
    vertex_data: *const u8,
    vertex_count: u32,
    index_data: *const u8,
    index_count: u32,
) -> HelioMeshUploadPtr {
    let vertices = unsafe {
        std::slice::from_raw_parts(
            vertex_data as *const PackedVertex,
            vertex_count as usize,
        )
    };
    let indices = unsafe {
        std::slice::from_raw_parts(
            index_data as *const u32,
            index_count as usize,
        )
    };
    let upload = MeshUpload {
        vertices: vertices.to_vec(),
        indices: indices.to_vec(),
    };
    Box::into_raw(Box::new(upload)) as HelioMeshUploadPtr
}

#[no_mangle]
pub unsafe extern "C" fn helio_mesh_upload_free(upload: HelioMeshUploadPtr) {
    if !upload.is_null() {
        drop(Box::from_raw(upload as *mut MeshUpload));
    }
}
