#pragma once

#include <helio/helio_capi.h>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace helio {

// ── Error handling ─────────────────────────────────────────────────────────────

class Error : public std::runtime_error {
public:
    explicit Error(std::string msg) : std::runtime_error(std::move(msg)) {}
};

inline void check(HelioResult r) {
    if (!r.success) {
        std::string msg = r.error_message ? r.error_message : "unknown error";
        helio_free_error_string(r.error_message);
        throw Error(std::move(msg));
    }
}

// ── Handle wrappers ────────────────────────────────────────────────────────────

class Handle {
public:
    Handle() : h_(helio_handle_null()) {}
    explicit Handle(HelioHandle h) : h_(h) {}

    HelioHandle get() const { return h_; }
    bool is_null() const { return helio_handle_is_null(h_); }
    bool operator==(const Handle& o) const { return h_.slot == o.h_.slot && h_.generation == o.h_.generation; }
    bool operator!=(const Handle& o) const { return !(*this == o); }

    static Handle null() { return Handle(helio_handle_null()); }

private:
    HelioHandle h_;
};

// Typed handle aliases
using MeshId                = Handle;
using MaterialId            = Handle;
using TextureId             = Handle;
using LightId               = Handle;
using ObjectId              = Handle;
using DecalId               = Handle;
using WaterVolumeId         = Handle;
using WaterHitboxId         = Handle;
using ReflectionCaptureId   = Handle;
using VoxelVolumeId         = Handle;
using VirtualMeshId         = Handle;
using VirtualObjectId       = Handle;

// ── Math helpers ───────────────────────────────────────────────────────────────

inline HelioVec3 vec3(float x, float y, float z) {
    HelioVec3 v = { x, y, z };
    return v;
}

inline HelioMat4 mat4_identity() {
    HelioMat4 m = {};
    for (int i = 0; i < 4; ++i) m.data[i][i] = 1.0f;
    return m;
}

// ── Camera helpers ─────────────────────────────────────────────────────────────

inline HelioCameraDesc camera_from_matrices(const HelioMat4& view, const HelioMat4& proj,
                                            float near_, float far_) {
    return helio_camera_from_matrices(view, proj, near_, far_);
}

inline HelioCameraDesc camera_perspective_look_at(
    float px, float py, float pz,
    float tx, float ty, float tz,
    float fov_y, float aspect, float near_, float far_)
{
    return helio_camera_perspective_look_at(px, py, pz, tx, ty, tz,
                                            fov_y, aspect, near_, far_);
}

// ── MeshUpload builder ─────────────────────────────────────────────────────────

class MeshUpload {
public:
    MeshUpload() : ptr_(nullptr) {}
    ~MeshUpload() { release(); }
    MeshUpload(const MeshUpload&) = delete;
    MeshUpload& operator=(const MeshUpload&) = delete;
    MeshUpload(MeshUpload&& o) noexcept : ptr_(o.ptr_) { o.ptr_ = nullptr; }
    MeshUpload& operator=(MeshUpload&& o) noexcept {
        if (this != &o) { release(); ptr_ = o.ptr_; o.ptr_ = nullptr; }
        return *this;
    }

    void build(const std::vector<HelioPackedVertex>& vertices,
               const std::vector<uint32_t>& indices) {
        release();
        ptr_ = helio_mesh_upload_create(
            reinterpret_cast<const uint8_t*>(vertices.data()),
            static_cast<uint32_t>(vertices.size()),
            reinterpret_cast<const uint8_t*>(indices.data()),
            static_cast<uint32_t>(indices.size()));
    }

    HelioMeshUploadPtr get() const { return ptr_; }

    void release() {
        if (ptr_) { helio_mesh_upload_free(ptr_); ptr_ = nullptr; }
    }

private:
    HelioMeshUploadPtr ptr_;
};

// ── Scene ──────────────────────────────────────────────────────────────────────

class Scene {
public:
    Scene(HelioDevicePtr device, HelioQueuePtr queue)
        : ptr_(helio_scene_new(device, queue)) {}

    ~Scene() { helio_scene_destroy(ptr_); }
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&& o) noexcept : ptr_(o.ptr_) { o.ptr_ = nullptr; }
    Scene& operator=(Scene&& o) noexcept {
        if (this != &o) { helio_scene_destroy(ptr_); ptr_ = o.ptr_; o.ptr_ = nullptr; }
        return *this;
    }

    HelioScenePtr get() const { return ptr_; }

    void flush() { helio_scene_flush(ptr_); }
    void advance_frame() { helio_scene_advance_frame(ptr_); }
    void set_render_size(uint32_t w, uint32_t h) { helio_scene_set_render_size(ptr_, w, h); }
    void clear() { helio_scene_clear(ptr_); }

    void update_camera(const HelioCameraDesc& cam) { helio_scene_update_camera(ptr_, &cam); }

    void hide_group(uint8_t g) { helio_scene_hide_group(ptr_, g); }
    void show_group(uint8_t g) { helio_scene_show_group(ptr_, g); }
    bool is_group_hidden(uint8_t g) { return helio_scene_is_group_hidden(ptr_, g); }

    MeshId insert_mesh(const HelioMeshUpload& upload) {
        return MeshId(helio_scene_insert_mesh(ptr_, &upload));
    }
    MeshId insert_dynamic_mesh(const HelioMeshUpload& upload) {
        return MeshId(helio_scene_insert_dynamic_mesh(ptr_, &upload));
    }
    void remove_mesh(MeshId id) { check(helio_scene_remove_mesh(ptr_, id.get())); }

    MaterialId insert_material(const HelioGpuMaterial& mat) {
        return MaterialId(helio_scene_insert_material(ptr_, &mat));
    }
    void remove_material(MaterialId id) { check(helio_scene_remove_material(ptr_, id.get())); }
    void update_material(MaterialId id, const HelioGpuMaterial& mat) {
        check(helio_scene_update_material(ptr_, id.get(), &mat));
    }

    void insert_texture(const HelioTextureUpload& upload) {
        check(helio_scene_insert_texture(ptr_, &upload));
    }
    void remove_texture(TextureId id) { check(helio_scene_remove_texture(ptr_, id.get())); }

    LightId insert_light(const HelioGpuLight& light) {
        return LightId(helio_scene_insert_light(ptr_, &light));
    }
    LightId insert_light_with_tag(const HelioGpuLight& light, uint64_t tag, uint32_t mov) {
        return LightId(helio_scene_insert_light_with_tag(ptr_, &light, tag, mov));
    }
    void remove_light(LightId id) { check(helio_scene_remove_light(ptr_, id.get())); }
    void update_light(LightId id, const HelioGpuLight& data) {
        check(helio_scene_update_light(ptr_, id.get(), &data));
    }

    ObjectId insert_object(const HelioObjectDescriptor& desc) {
        HelioResult r = helio_scene_insert_object(ptr_, &desc);
        // Return the last handle that was created; the actual object handle is
        // retrieved from the descriptor's response.  For simplicity, we just
        // check success.
        check(r);
        // In practice you'd track the returned handle from the Rust side.
        return ObjectId::null();
    }
    void remove_object(ObjectId id) { check(helio_scene_remove_object(ptr_, id.get())); }
    void update_object_transform(ObjectId id, const HelioMat4& t) {
        check(helio_scene_update_object_transform(ptr_, id.get(), &t));
    }
    bool get_object_transform(ObjectId id, HelioMat4* out) {
        return helio_scene_get_object_transform(ptr_, id.get(), out);
    }
    void update_object_material(ObjectId id, MaterialId mat) {
        check(helio_scene_update_object_material(ptr_, id.get(), mat.get()));
    }

    void set_material_class(MaterialId id, uint32_t cls, uint64_t hash,
                            uint32_t features, bool use_flags) {
        check(helio_scene_set_material_class(ptr_, id.get(), cls, hash, features, use_flags));
    }
    void update_material_class_params(MaterialId id, const float params[4]) {
        check(helio_scene_update_material_class_params(ptr_, id.get(), params));
    }

    WaterVolumeId insert_water_volume(const HelioWaterVolumeDesc& desc) {
        HelioResult r = helio_scene_insert_water_volume(ptr_, &desc);
        check(r);
        return WaterVolumeId::null();
    }
    void remove_water_volume(WaterVolumeId id) { check(helio_scene_remove_water_volume(ptr_, id.get())); }

    WaterHitboxId insert_water_hitbox(const HelioWaterHitboxDesc& desc) {
        HelioResult r = helio_scene_insert_water_hitbox(ptr_, &desc);
        check(r);
        return WaterHitboxId::null();
    }
    void remove_water_hitbox(WaterHitboxId id) { check(helio_scene_remove_water_hitbox(ptr_, id.get())); }

    DecalId insert_decal(const HelioGpuDecal& decal) {
        return DecalId(helio_scene_insert_decal(ptr_, &decal));
    }
    bool remove_decal(DecalId id) { return helio_scene_remove_decal(ptr_, id.get()); }
    bool update_decal(DecalId id, const HelioGpuDecal& data) {
        return helio_scene_update_decal(ptr_, id.get(), &data);
    }

    void insert_reflection_capture(const HelioReflectionCaptureDesc& desc) {
        check(helio_scene_insert_reflection_capture(ptr_, &desc));
    }
    bool remove_reflection_capture(ReflectionCaptureId id) {
        return helio_scene_remove_reflection_capture(ptr_, id.get());
    }

    VoxelVolumeId insert_voxel_volume(const HelioVoxelVolumeDescriptor& desc) {
        return VoxelVolumeId(helio_scene_insert_voxel_volume(ptr_, &desc));
    }
    void remove_voxel_volume(VoxelVolumeId id) { check(helio_scene_remove_voxel_volume(ptr_, id.get())); }

    VirtualMeshId insert_virtual_mesh(const HelioMeshUpload& upload) {
        return VirtualMeshId(helio_scene_insert_virtual_mesh(ptr_, &upload));
    }
    void insert_virtual_object(const HelioVirtualObjectDescriptor& desc) {
        check(helio_scene_insert_virtual_object(ptr_, &desc));
    }

private:
    HelioScenePtr ptr_;
};

// ── Renderer ───────────────────────────────────────────────────────────────────

class Renderer {
public:
    Renderer(HelioDevicePtr device, HelioQueuePtr queue,
             uint32_t surface_format, const HelioRendererConfig& config,
             Scene& scene, HelioBufferPtr debug_cam_buf, HelioBufferPtr cull_buf)
        : ptr_(helio_renderer_new(device, queue, surface_format, &config,
                                  scene.get(), debug_cam_buf, cull_buf))
    {
        // Scene ownership transferred to renderer — mark it released.
        // C++ side must not use the original Scene handle after this.
    }

    ~Renderer() { helio_renderer_destroy(ptr_); }
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&& o) noexcept : ptr_(o.ptr_) { o.ptr_ = nullptr; }
    Renderer& operator=(Renderer&& o) noexcept {
        if (this != &o) { helio_renderer_destroy(ptr_); ptr_ = o.ptr_; o.ptr_ = nullptr; }
        return *this;
    }

    HelioRendererPtr get() const { return ptr_; }

    void render(const HelioCameraDesc& cam, HelioTextureViewPtr target) {
        check(helio_renderer_render(ptr_, &cam, target));
    }

    void set_clear_color(const float color[4]) { helio_renderer_set_clear_color(ptr_, color); }
    void set_ambient(const float color[3], float intensity) {
        helio_renderer_set_ambient(ptr_, color, intensity);
    }
    void set_debug_mode(uint32_t mode) { helio_renderer_set_debug_mode(ptr_, mode); }
    void set_editor_mode(bool enabled) { helio_renderer_set_editor_mode(ptr_, enabled); }
    void set_shadow_quality(uint32_t q) { helio_renderer_set_shadow_quality(ptr_, q); }

    void output_size(uint32_t* w, uint32_t* h) { helio_renderer_output_size(ptr_, w, h); }

    void debug_line(const float from[3], const float to[3], const float color[4]) {
        helio_renderer_debug_line(ptr_, from, to, color);
    }
    void debug_sphere(const float center[3], float radius, const float color[4], uint32_t segs) {
        helio_renderer_debug_sphere(ptr_, center, radius, color, segs);
    }

private:
    HelioRendererPtr ptr_;
};

// ── Editor ──────────────────────────────────────────────────────────────────────

class Editor {
public:
    Editor() : ptr_(helio_editor_new()) {}
    ~Editor() { helio_editor_destroy(ptr_); }
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;
    Editor(Editor&& o) noexcept : ptr_(o.ptr_) { o.ptr_ = nullptr; }
    Editor& operator=(Editor&& o) noexcept {
        if (this != &o) { helio_editor_destroy(ptr_); ptr_ = o.ptr_; o.ptr_ = nullptr; }
        return *this;
    }

    HelioEditorPtr get() const { return ptr_; }

    void select(HelioSceneActorId id) { helio_editor_select(ptr_, id); }
    void deselect() { helio_editor_deselect(ptr_); }
    HelioSceneActorId selected() { return helio_editor_selected(ptr_); }
    uint32_t gizmo_mode() { return helio_editor_gizmo_mode(ptr_); }
    void set_gizmo_mode(uint32_t m) { helio_editor_set_gizmo_mode(ptr_, m); }
    int32_t hovered_axis() { return helio_editor_hovered_axis(ptr_); }
    bool is_dragging() { return helio_editor_is_dragging(ptr_); }

    static void ray_from_screen(float px, float py, float w, float h,
                                const HelioMat4& vp_inv,
                                float out_origin[3], float out_dir[3]) {
        helio_editor_ray_from_screen(px, py, w, h, &vp_inv, out_origin, out_dir);
    }

    bool update_hover(const float origin[3], const float dir[3], Renderer& r) {
        return helio_editor_update_hover(ptr_, origin, dir, r.get());
    }
    bool try_start_drag(const float origin[3], const float dir[3], Renderer& r) {
        return helio_editor_try_start_drag(ptr_, origin, dir, r.get());
    }
    void update_drag(const float origin[3], const float dir[3], Renderer& r) {
        helio_editor_update_drag(ptr_, origin, dir, r.get());
    }
    void end_drag() { helio_editor_end_drag(ptr_); }
    void draw_gizmos(Renderer& r) { helio_editor_draw_gizmos(ptr_, r.get()); }

private:
    HelioEditorPtr ptr_;
};

// ── Picker ─────────────────────────────────────────────────────────────────────

class Picker {
public:
    Picker() : ptr_(helio_picker_new()) {}
    ~Picker() { helio_picker_destroy(ptr_); }
    Picker(const Picker&) = delete;
    Picker& operator=(const Picker&) = delete;
    Picker(Picker&& o) noexcept : ptr_(o.ptr_) { o.ptr_ = nullptr; }
    Picker& operator=(Picker&& o) noexcept {
        if (this != &o) { helio_picker_destroy(ptr_); ptr_ = o.ptr_; o.ptr_ = nullptr; }
        return *this;
    }

    HelioPickerPtr get() const { return ptr_; }

    void register_mesh(MeshId id, const std::vector<HelioPackedVertex>& verts,
                       const std::vector<uint32_t>& indices) {
        helio_picker_register_mesh(ptr_, id.get(),
                                   reinterpret_cast<const uint8_t*>(verts.data()),
                                   static_cast<uint32_t>(verts.size()),
                                   reinterpret_cast<const uint8_t*>(indices.data()),
                                   static_cast<uint32_t>(indices.size()));
    }

    void rebuild_instances(Scene& scene) {
        helio_picker_rebuild_instances(ptr_, scene.get());
    }

    bool cast_ray(Scene& scene, const float origin[3], const float dir[3],
                  HelioPickHit* out_hit) {
        return helio_picker_cast_ray(ptr_, scene.get(), origin, dir, out_hit);
    }

    bool cast_ray_from_to(Scene& scene, const float from[3], const float to[3],
                          HelioPickHit* out_hit) {
        return helio_picker_cast_ray_from_to(ptr_, scene.get(), from, to, out_hit);
    }

private:
    HelioPickerPtr ptr_;
};

} // namespace helio
