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

/// Create a perspective look-at camera.
#[no_mangle]
pub unsafe extern "C" fn helio_camera_perspective_look_at(
    position: HelioVec3,
    target: HelioVec3,
    up: HelioVec3,
    fov_y_radians: f32,
    aspect: f32,
    near: f32,
    far: f32,
) -> HelioCameraDesc {
    let glam_pos: glam::Vec3 = position.into();
    let glam_target: glam::Vec3 = target.into();
    let glam_up: glam::Vec3 = up.into();
    let view = glam::Mat4::look_at_rh(glam_pos, glam_target, glam_up);
    let proj = glam::Mat4::perspective_rh(fov_y_radians, aspect, near, far);
    HelioCameraDesc {
        view: view.into(),
        proj: proj.into(),
        position,
        near,
        far,
        jitter: [0.0, 0.0],
    }
}

/// Convert a HelioCameraDesc to a Rust Camera.
pub(crate) fn camera_from_desc(desc: &HelioCameraDesc) -> Camera {
    Camera::from_matrices(
        desc.view.into(),
        desc.proj.into(),
        desc.position.into(),
        desc.near,
        desc.far,
    )
}
