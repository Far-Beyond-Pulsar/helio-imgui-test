use helio::{EditorState, GizmoMode, GizmoAxis, Renderer};

use crate::types::*;

#[no_mangle]
pub unsafe extern "C" fn helio_editor_new() -> HelioEditorPtr {
    Box::into_raw(Box::new(EditorState::new())) as HelioEditorPtr
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_destroy(editor: HelioEditorPtr) {
    if !editor.is_null() {
        drop(Box::from_raw(editor as *mut EditorState));
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_select(
    editor: HelioEditorPtr,
    actor_id: HelioSceneActorId,
) {
    let editor = &mut *(editor as *mut EditorState);
    editor.select(actor_id.into());
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_deselect(editor: HelioEditorPtr) {
    let editor = &mut *(editor as *mut EditorState);
    editor.deselect();
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_selected(
    editor: HelioEditorPtr,
) -> HelioSceneActorId {
    let editor = &*(editor as *const EditorState);
    match editor.selected() {
        Some(id) => id.into(),
        None => HelioSceneActorId::none(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_gizmo_mode(
    editor: HelioEditorPtr,
) -> u32 {
    let editor = &*(editor as *const EditorState);
    match editor.gizmo_mode() {
        GizmoMode::Translate => 0,
        GizmoMode::Rotate => 1,
        GizmoMode::Scale => 2,
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_set_gizmo_mode(
    editor: HelioEditorPtr,
    mode: u32,
) {
    let editor = &mut *(editor as *mut EditorState);
    let m = match mode {
        1 => GizmoMode::Rotate,
        2 => GizmoMode::Scale,
        _ => GizmoMode::Translate,
    };
    editor.set_gizmo_mode(m);
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_hovered_axis(
    editor: HelioEditorPtr,
) -> i32 {
    let editor = &*(editor as *const EditorState);
    match editor.hovered_axis() {
        Some(GizmoAxis::X) => 0,
        Some(GizmoAxis::Y) => 1,
        Some(GizmoAxis::Z) => 2,
        None => -1,
    }
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_is_dragging(editor: HelioEditorPtr) -> bool {
    let editor = &*(editor as *const EditorState);
    editor.is_dragging()
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_ray_from_screen(
    px: f32,
    py: f32,
    width: f32,
    height: f32,
    view_proj_inv: &HelioMat4,
    out_origin: &mut [f32; 3],
    out_dir: &mut [f32; 3],
) {
    let (o, d) = EditorState::ray_from_screen(
        px, py, width, height,
        (*view_proj_inv).into(),
    );
    *out_origin = o.into();
    *out_dir = d.into();
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_update_hover(
    editor: HelioEditorPtr,
    ray_origin: &[f32; 3],
    ray_dir: &[f32; 3],
    renderer: HelioRendererPtr,
) -> bool {
    let editor = &mut *(editor as *mut EditorState);
    let renderer = &mut *(renderer as *mut Renderer);
    editor.update_hover((*ray_origin).into(), (*ray_dir).into(), renderer)
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_try_start_drag(
    editor: HelioEditorPtr,
    ray_origin: &[f32; 3],
    ray_dir: &[f32; 3],
    renderer: HelioRendererPtr,
) -> bool {
    let editor = &mut *(editor as *mut EditorState);
    let renderer = &mut *(renderer as *mut Renderer);
    editor.try_start_drag((*ray_origin).into(), (*ray_dir).into(), renderer.scene())
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_update_drag(
    editor: HelioEditorPtr,
    ray_origin: &[f32; 3],
    ray_dir: &[f32; 3],
    renderer: HelioRendererPtr,
) {
    let editor = &mut *(editor as *mut EditorState);
    let renderer = &mut *(renderer as *mut Renderer);
    editor.update_drag((*ray_origin).into(), (*ray_dir).into(), renderer);
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_end_drag(editor: HelioEditorPtr) {
    let editor = &mut *(editor as *mut EditorState);
    editor.end_drag();
}

#[no_mangle]
pub unsafe extern "C" fn helio_editor_draw_gizmos(
    editor: HelioEditorPtr,
    renderer: HelioRendererPtr,
) {
    let editor = &*(editor as *const EditorState);
    let renderer = &mut *(renderer as *mut Renderer);
    editor.draw_gizmos(renderer);
}
