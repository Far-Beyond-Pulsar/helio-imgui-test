#ifndef HELIO_CAPI_H
#define HELIO_CAPI_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── Opaque pointer types ───────────────────────────────────────────────────────

typedef void* HelioScenePtr;
typedef void* HelioRendererPtr;
typedef void* HelioPickerPtr;
typedef void* HelioEditorPtr;
typedef void* HelioDevicePtr;
typedef void* HelioQueuePtr;
typedef void* HelioTextureViewPtr;
typedef void* HelioBufferPtr;
typedef void* HelioMeshUploadPtr;

// ── Error result ───────────────────────────────────────────────────────────────

typedef struct HelioResult {
    bool success;
    char* error_message;
} HelioResult;

void helio_free_error_string(char* s);

// ── Math types ─────────────────────────────────────────────────────────────────

typedef struct HelioVec3 {
    float x, y, z;
} HelioVec3;

typedef struct HelioMat4 {
    float data[4][4];
} HelioMat4;

// ── Handle ─────────────────────────────────────────────────────────────────────

typedef struct HelioHandle {
    uint32_t slot;
    uint32_t generation;
} HelioHandle;

static inline HelioHandle helio_handle_null(void) {
    HelioHandle h = { UINT32_MAX, 0 };
    return h;
}

static inline bool helio_handle_is_null(HelioHandle h) {
    return h.slot == UINT32_MAX;
}

// ── Actor ID ───────────────────────────────────────────────────────────────────

typedef enum HelioSceneActorType {
    HELIO_ACTOR_NONE              = 0,
    HELIO_ACTOR_DECAL             = 1,
    HELIO_ACTOR_MESH              = 2,
    HELIO_ACTOR_LIGHT             = 3,
    HELIO_ACTOR_REFLECTION_CAPTURE = 4,
    HELIO_ACTOR_VIRTUAL_MESH      = 5,
    HELIO_ACTOR_VIRTUAL_OBJECT    = 6,
    HELIO_ACTOR_OBJECT            = 7,
    HELIO_ACTOR_SECTIONED_OBJECT  = 8,
    HELIO_ACTOR_WATER_VOLUME      = 9,
    HELIO_ACTOR_WATER_HITBOX      = 10,
    HELIO_ACTOR_POST_PROCESS_VOLUME = 11,
} HelioSceneActorType;

typedef struct HelioSceneActorId {
    HelioSceneActorType actor_type;
    HelioHandle handle;
} HelioSceneActorId;

// ── Pick hit ───────────────────────────────────────────────────────────────────

typedef struct HelioPickHit {
    HelioSceneActorId actor_id;
    float t;
    HelioVec3 position;
    HelioVec3 normal;
    uint64_t user_tag;
} HelioPickHit;

// ── Camera ─────────────────────────────────────────────────────────────────────

typedef struct HelioCameraDesc {
    HelioMat4 view;
    HelioMat4 proj;
    HelioVec3 position;
    float near;
    float far;
    float jitter[2];
} HelioCameraDesc;

// ── Lights ─────────────────────────────────────────────────────────────────────

typedef struct HelioGpuLight {
    float position_range[4];
    float direction_outer[4];
    float color_intensity[4];
    uint32_t shadow_index;
    uint32_t light_type;
    float inner_angle;
    uint32_t _pad;
    uint32_t god_rays_enabled;
    float god_rays_density;
    float god_rays_weight;
    float god_rays_decay;
    float god_rays_exposure;
    uint32_t _pad2[3];
} HelioGpuLight;

// ── Materials ──────────────────────────────────────────────────────────────────

typedef struct HelioGpuMaterial {
    float base_color[4];
    float emissive[4];
    float roughness_metallic[4];
    uint32_t tex_base_color;
    uint32_t tex_normal;
    uint32_t tex_roughness;
    uint32_t tex_emissive;
    uint32_t tex_occlusion;
    uint32_t workflow;
    uint32_t flags;
    uint32_t material_class;
    float class_params[4];
} HelioGpuMaterial;

// ── Mesh upload ────────────────────────────────────────────────────────────────

typedef struct HelioPackedVertex {
    float position[3];
    float bitangent_sign;
    float tex_coords0[2];
    float tex_coords1[2];
    uint32_t normal;
    uint32_t tangent;
} HelioPackedVertex;

typedef struct HelioMeshUpload {
    const HelioPackedVertex* vertices;
    uint32_t vertex_count;
    const uint32_t* indices;
    uint32_t index_count;
} HelioMeshUpload;

// ── Decals ─────────────────────────────────────────────────────────────────────

typedef struct HelioGpuDecal {
    float transform[16];
    float color[4];
    uint32_t albedo_texture_index;
    uint32_t normal_texture_index;
    uint32_t roughness_texture_index;
    uint32_t metalness_texture_index;
    uint32_t blend_mode;
    uint32_t decal_type;
    float fade_time;
    float fade_start_delay;
    float age;
    uint32_t normal_adapt;
    float _pad0;
    float _pad1;
} HelioGpuDecal;

// ── Water ──────────────────────────────────────────────────────────────────────

typedef struct HelioWaterVolumeDesc {
    float bounds_min[3];
    float bounds_max[3];
    float surface_height;
    float wave_amplitude;
    float wave_frequency;
    float wave_speed;
    float wave_direction[2];
    float wave_steepness;
    float water_color[3];
    float extinction[3];
    float foam_threshold;
    float foam_amount;
    float reflection_strength;
    float refraction_strength;
    float fresnel_power;
    bool caustics_enabled;
    float caustics_intensity;
    float caustics_scale;
    float caustics_speed;
    float fog_density;
    float god_rays_intensity;
    bool ssr_enabled;
    uint32_t ssr_steps;
    float ssr_step_size;
    float ssr_thickness;
    float ior;
    float fresnel_min;
    float density;
    float shadow_rim;
    float shadow_hitbox;
    float shadow_ao;
    float sun_direction[3];
    float wave_spring;
    float wave_damping;
    float wind_direction[2];
    float wind_strength;
    float wave_scale;
} HelioWaterVolumeDesc;

typedef struct HelioWaterHitboxDesc {
    float old_min[3];
    float old_max[3];
    float new_min[3];
    float new_max[3];
    float edge_softness;
    float strength;
} HelioWaterHitboxDesc;

// ── Reflection captures ────────────────────────────────────────────────────────

typedef struct HelioReflectionCaptureDesc {
    uint32_t shape;
    uint32_t mobility;
    HelioMat4 transform;
    float influence_radius;
    float extents[3];
    float transition_distance;
    float brightness;
} HelioReflectionCaptureDesc;

// ── Voxels ─────────────────────────────────────────────────────────────────────

typedef struct HelioVoxelVolumeDescriptor {
    float voxel_size;
    float root_extent;
    HelioMat4 local_to_world;
    uint32_t movability;
    uint32_t mode;
} HelioVoxelVolumeDescriptor;

// ── Objects ────────────────────────────────────────────────────────────────────

typedef struct HelioObjectDescriptor {
    HelioHandle mesh;
    HelioHandle material;
    HelioMat4 transform;
    float bounds[4];
    uint32_t flags;
    uint64_t groups;
    uint32_t movability;
    uint64_t user_tag;
} HelioObjectDescriptor;

typedef struct HelioVirtualObjectDescriptor {
    HelioHandle virtual_mesh;
    uint32_t material_id;
    HelioMat4 transform;
    float bounds[4];
    uint32_t flags;
    uint64_t groups;
    uint32_t movability;
} HelioVirtualObjectDescriptor;

// ── Renderer config ────────────────────────────────────────────────────────────

typedef struct HelioRendererConfig {
    uint32_t width;
    uint32_t height;
    uint32_t surface_format;
    float gi_rc_radius;
    float gi_fade_margin;
    uint32_t shadow_quality;
    uint32_t debug_mode;
    float render_scale;
    uint32_t perf_overlay_mode;
    uint32_t shadow_atlas_size;
    uint32_t shadow_face_capacity;
} HelioRendererConfig;

// ── Texture upload ─────────────────────────────────────────────────────────────

typedef struct HelioTextureUpload {
    const char* label;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    const uint8_t* data;
    size_t data_len;
    uint32_t address_mode_u;
    uint32_t address_mode_v;
    uint32_t address_mode_w;
    uint32_t mag_filter;
    uint32_t min_filter;
    uint32_t mipmap_filter;
} HelioTextureUpload;

// ── Meshlet ────────────────────────────────────────────────────────────────────

typedef struct HelioGpuMeshletEntry {
    float center[3];
    float radius;
    float cone_apex[3];
    float cone_cutoff;
    float cone_axis[3];
    float lod_error;
    uint32_t first_index;
    uint32_t index_count;
    int32_t vertex_offset;
    uint32_t instance_index;
} HelioGpuMeshletEntry;

// ════════════════════════════════════════════════════════════════════════════════
// Camera
// ════════════════════════════════════════════════════════════════════════════════

HelioCameraDesc helio_camera_from_matrices(HelioMat4 view, HelioMat4 proj,
                                           float near, float far);
HelioCameraDesc helio_camera_perspective_look_at(
    float px, float py, float pz,
    float tx, float ty, float tz,
    float fov_y_radians, float aspect, float near, float far);

// ════════════════════════════════════════════════════════════════════════════════
// Scene
// ════════════════════════════════════════════════════════════════════════════════

HelioScenePtr helio_scene_new(HelioDevicePtr device, HelioQueuePtr queue);
void helio_scene_destroy(HelioScenePtr scene);
void helio_scene_flush(HelioScenePtr scene);
void helio_scene_advance_frame(HelioScenePtr scene);
void helio_scene_set_render_size(HelioScenePtr scene, uint32_t width, uint32_t height);
void helio_scene_clear(HelioScenePtr scene);

void helio_scene_update_camera(HelioScenePtr scene, const HelioCameraDesc* camera);

void helio_scene_hide_group(HelioScenePtr scene, uint8_t group_index);
void helio_scene_show_group(HelioScenePtr scene, uint8_t group_index);
bool helio_scene_is_group_hidden(HelioScenePtr scene, uint8_t group_index);

HelioHandle helio_scene_insert_mesh(HelioScenePtr scene, const HelioMeshUpload* upload);
HelioHandle helio_scene_insert_dynamic_mesh(HelioScenePtr scene, const HelioMeshUpload* upload);
HelioResult helio_scene_remove_mesh(HelioScenePtr scene, HelioHandle mesh);

HelioHandle helio_scene_insert_material(HelioScenePtr scene, const HelioGpuMaterial* material);
HelioResult helio_scene_remove_material(HelioScenePtr scene, HelioHandle material);
HelioResult helio_scene_update_material(HelioScenePtr scene, HelioHandle material,
                                        const HelioGpuMaterial* data);

HelioResult helio_scene_insert_texture(HelioScenePtr scene, const HelioTextureUpload* upload);
HelioResult helio_scene_remove_texture(HelioScenePtr scene, HelioHandle texture);

HelioHandle helio_scene_insert_light(HelioScenePtr scene, const HelioGpuLight* light);
HelioHandle helio_scene_insert_light_with_tag(HelioScenePtr scene, const HelioGpuLight* light,
                                              uint64_t user_tag, uint32_t movability);
HelioResult helio_scene_remove_light(HelioScenePtr scene, HelioHandle light);
HelioResult helio_scene_update_light(HelioScenePtr scene, HelioHandle light,
                                     const HelioGpuLight* data);

HelioResult helio_scene_insert_object(HelioScenePtr scene, const HelioObjectDescriptor* desc);
HelioResult helio_scene_remove_object(HelioScenePtr scene, HelioHandle object);
HelioResult helio_scene_update_object_transform(HelioScenePtr scene, HelioHandle object,
                                                const HelioMat4* transform);
bool helio_scene_get_object_transform(HelioScenePtr scene, HelioHandle object,
                                      HelioMat4* out_transform);
HelioResult helio_scene_update_object_material(HelioScenePtr scene, HelioHandle object,
                                               HelioHandle material);

HelioResult helio_scene_set_material_class(HelioScenePtr scene, HelioHandle material,
                                           uint32_t material_class, uint64_t graph_hash,
                                           uint32_t feature_flags, bool use_flags);
HelioResult helio_scene_update_material_class_params(HelioScenePtr scene, HelioHandle material,
                                                     const float params[4]);

// Water
HelioResult helio_scene_insert_water_volume(HelioScenePtr scene, const HelioWaterVolumeDesc* desc);
HelioResult helio_scene_remove_water_volume(HelioScenePtr scene, HelioHandle volume);
HelioResult helio_scene_insert_water_hitbox(HelioScenePtr scene, const HelioWaterHitboxDesc* desc);
HelioResult helio_scene_remove_water_hitbox(HelioScenePtr scene, HelioHandle hitbox);

// Decals
HelioHandle helio_scene_insert_decal(HelioScenePtr scene, const HelioGpuDecal* decal);
bool helio_scene_remove_decal(HelioScenePtr scene, HelioHandle decal);
bool helio_scene_update_decal(HelioScenePtr scene, HelioHandle decal, const HelioGpuDecal* data);

// Reflection captures
HelioResult helio_scene_insert_reflection_capture(HelioScenePtr scene,
                                                  const HelioReflectionCaptureDesc* desc);
bool helio_scene_remove_reflection_capture(HelioScenePtr scene, HelioHandle capture);

// Voxel volumes
HelioHandle helio_scene_insert_voxel_volume(HelioScenePtr scene,
                                            const HelioVoxelVolumeDescriptor* desc);
HelioResult helio_scene_remove_voxel_volume(HelioScenePtr scene, HelioHandle volume);

// Virtual geometry
HelioHandle helio_scene_insert_virtual_mesh(HelioScenePtr scene, const HelioMeshUpload* upload);
HelioResult helio_scene_insert_virtual_object(HelioScenePtr scene,
                                              const HelioVirtualObjectDescriptor* desc);

// ════════════════════════════════════════════════════════════════════════════════
// Renderer
// ════════════════════════════════════════════════════════════════════════════════

HelioRendererPtr helio_renderer_new(HelioDevicePtr device, HelioQueuePtr queue,
                                    uint32_t surface_format, const HelioRendererConfig* config,
                                    HelioScenePtr scene, HelioBufferPtr debug_camera_buffer,
                                    HelioBufferPtr cull_stats_buffer);
void helio_renderer_destroy(HelioRendererPtr renderer);
HelioResult helio_renderer_render(HelioRendererPtr renderer, const HelioCameraDesc* camera,
                                  HelioTextureViewPtr target);
HelioScenePtr helio_renderer_scene(HelioRendererPtr renderer);
void helio_renderer_set_clear_color(HelioRendererPtr renderer, const float color[4]);
void helio_renderer_set_ambient(HelioRendererPtr renderer, const float color[3], float intensity);
void helio_renderer_set_debug_mode(HelioRendererPtr renderer, uint32_t mode);
void helio_renderer_set_editor_mode(HelioRendererPtr renderer, bool enabled);
void helio_renderer_set_shadow_quality(HelioRendererPtr renderer, uint32_t quality);
void helio_renderer_output_size(HelioRendererPtr renderer, uint32_t* out_width, uint32_t* out_height);
void helio_renderer_debug_line(HelioRendererPtr renderer, const float from[3],
                               const float to[3], const float color[4]);
void helio_renderer_debug_sphere(HelioRendererPtr renderer, const float center[3],
                                 float radius, const float color[4], uint32_t segments);

// ════════════════════════════════════════════════════════════════════════════════
// Editor
// ════════════════════════════════════════════════════════════════════════════════

HelioEditorPtr helio_editor_new(void);
void helio_editor_destroy(HelioEditorPtr editor);
void helio_editor_select(HelioEditorPtr editor, HelioSceneActorId actor_id);
void helio_editor_deselect(HelioEditorPtr editor);
HelioSceneActorId helio_editor_selected(HelioEditorPtr editor);
uint32_t helio_editor_gizmo_mode(HelioEditorPtr editor);
void helio_editor_set_gizmo_mode(HelioEditorPtr editor, uint32_t mode);
int32_t helio_editor_hovered_axis(HelioEditorPtr editor);
bool helio_editor_is_dragging(HelioEditorPtr editor);

void helio_editor_ray_from_screen(float px, float py, float width, float height,
                                  const HelioMat4* view_proj_inv,
                                  float out_origin[3], float out_dir[3]);
bool helio_editor_update_hover(HelioEditorPtr editor, const float ray_origin[3],
                               const float ray_dir[3], HelioRendererPtr renderer);
bool helio_editor_try_start_drag(HelioEditorPtr editor, const float ray_origin[3],
                                 const float ray_dir[3], HelioRendererPtr renderer);
void helio_editor_update_drag(HelioEditorPtr editor, const float ray_origin[3],
                              const float ray_dir[3], HelioRendererPtr renderer);
void helio_editor_end_drag(HelioEditorPtr editor);
void helio_editor_draw_gizmos(HelioEditorPtr editor, HelioRendererPtr renderer);

// ════════════════════════════════════════════════════════════════════════════════
// Picker
// ════════════════════════════════════════════════════════════════════════════════

HelioPickerPtr helio_picker_new(void);
void helio_picker_destroy(HelioPickerPtr picker);
void helio_picker_register_mesh(HelioPickerPtr picker, HelioHandle mesh_id,
                                const uint8_t* vertex_data, uint32_t vertex_count,
                                const uint8_t* index_data, uint32_t index_count);
void helio_picker_rebuild_instances(HelioPickerPtr picker, HelioScenePtr scene);
bool helio_picker_cast_ray(HelioPickerPtr picker, HelioScenePtr scene,
                           const float origin[3], const float dir[3],
                           HelioPickHit* out_hit);
bool helio_picker_cast_ray_from_to(HelioPickerPtr picker, HelioScenePtr scene,
                                   const float from[3], const float to[3],
                                   HelioPickHit* out_hit);

// ════════════════════════════════════════════════════════════════════════════════
// Mesh upload helpers
// ════════════════════════════════════════════════════════════════════════════════

HelioMeshUploadPtr helio_mesh_upload_create(const uint8_t* vertex_data, uint32_t vertex_count,
                                            const uint8_t* index_data, uint32_t index_count);
void helio_mesh_upload_free(HelioMeshUploadPtr upload);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // HELIO_CAPI_H
