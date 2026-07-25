use crate::types::*;
use helio::Camera;

/// Create a camera from view/projection matrices.
#[no_mangle]
pub unsafe extern "C" fn helio_camera_from_matrices(
    view: HelioMat4,
    proj: HelioMat4,
    position: HelioVec3,
    near: f32,
    far: f32,
) -> HelioCameraDesc {
    HelioCameraDesc {
        view,
        proj,
        position,
        near,
        far,
        jitter: [0.0, 0.0],
    }
}

/// Create a perspective look-at camera (no up — defaults to (0,1,0)).
#[no_mangle]
pub unsafe extern "C" fn helio_camera_perspective_look_at(
    px: f32, py: f32, pz: f32,
    tx: f32, ty: f32, tz: f32,
    fov_y_radians: f32,
    aspect: f32,
    near: f32,
    far: f32,
) -> HelioCameraDesc {
    let glam_pos = glam::Vec3::new(px, py, pz);
    let glam_target = glam::Vec3::new(tx, ty, tz);
    let glam_up = glam::Vec3::Y;
    let view = glam::Mat4::look_at_rh(glam_pos, glam_target, glam_up);
    let proj = glam::Mat4::perspective_rh(fov_y_radians, aspect, near, far);
    HelioCameraDesc {
        view: view.into(),
        proj: proj.into(),
        position: HelioVec3 { x: px, y: py, z: pz },
        near,
        far,
        jitter: [0.0, 0.0],
    }
}

/// Convert a HelioCameraDesc to a Rust Camera.
pub fn camera_from_desc(desc: &HelioCameraDesc) -> Camera {
    Camera::from_matrices(
        desc.view.into(),
        desc.proj.into(),
        desc.position.into(),
        desc.near,
        desc.far,
    )
}
