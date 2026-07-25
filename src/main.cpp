/// Helio ImGui Test App
///
/// Exercises the C FFI bindings (helio_ffi) by creating a window, initialising
/// wgpu via the built-in bootstrap helper, building a simple scene, and
/// rendering it with a Dear ImGui overlay.
///
/// Build + run with:  just run

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#undef near
#undef far

#include "bootstrap.h"
#include <helio/helio_capi.h>

// ── Helpers ────────────────────────────────────────────────────────────────────

static void fail(const char* msg) {
    fprintf(stderr, "FATAL: %s\n", msg);
    exit(1);
}

static HelioMat4 identity() {
    HelioMat4 m{};
    m.data[0][0] = 1.0f;
    m.data[1][1] = 1.0f;
    m.data[2][2] = 1.0f;
    m.data[3][3] = 1.0f;
    return m;
}

// ── Main ───────────────────────────────────────────────────────────────────────

int main() {
    // ── Window ────────────────────────────────────────────────────────────█
    glfwSetErrorCallback([](int e, const char* d) { fprintf(stderr, "GLFW %d: %s\n", e, d); });
    if (!glfwInit()) fail("glfwInit");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Helio ImGui Test", nullptr, nullptr);
    if (!window) fail("glfwCreateWindow");

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    // ── Bootstrap wgpu (device, queue, surface) ───────────────────────────█
    printf("[helio] Bootstrapping wgpu...\n");

    void* device_ptr  = nullptr;
    void* queue_ptr   = nullptr;
    void* debug_cam   = nullptr;
    void* cull_stats  = nullptr;

    if (!bootstrap_init((uint32_t)w, (uint32_t)h, &device_ptr, &queue_ptr,
                        &debug_cam, &cull_stats))
        fail("bootstrap_init");

    HWND hwnd = glfwGetWin32Window(window);
    HINSTANCE hinstance = (HINSTANCE)GetModuleHandleW(nullptr);

    if (!bootstrap_create_surface((void*)hinstance, (void*)hwnd))
        fail("bootstrap_create_surface");

    printf("[helio] device=%p queue=%p\n", device_ptr, queue_ptr);

    // ── Scene ─────────────────────────────────────────────────────────────█
    printf("[helio] Creating scene...\n");
    HelioScenePtr scene = helio_scene_new(device_ptr, queue_ptr);
    printf("[helio] scene=%p\n", (void*)scene);

    // Camera
    HelioCameraDesc cam = helio_camera_perspective_look_at(
        5, 3, 5,          // eye
        0, 0, 0,          // target
        1.0472f,          // 60 deg FOV
        (float)w / (float)h,
        0.1f, 100.0f
    );
    helio_scene_update_camera(scene, &cam);

    // ── Mesh (triangle) ───────────────────────────────────────────────────█
    printf("[helio] Creating mesh...\n");
    std::vector<HelioPackedVertex> verts = {
        { { 0, 1, 0 }, 0, { 0.5f, 1 }, { 0, 0 }, 0x7F0000, 0x7F0000 },
        { { -1, -1, 0 }, 0, { 0, 0 }, { 0, 0 }, 0x007F00, 0x007F00 },
        { { 1, -1, 0 }, 0, { 1, 0 }, { 0, 0 }, 0x00007F, 0x00007F },
    };
    std::vector<uint32_t> idx = { 0, 1, 2 };

    HelioMeshUpload upload;
    upload.vertices     = verts.data();
    upload.vertex_count = (uint32_t)verts.size();
    upload.indices      = idx.data();
    upload.index_count  = (uint32_t)idx.size();

    HelioHandle mesh_id = helio_scene_insert_mesh(scene, &upload);
    printf("[helio] mesh: slot=%u gen=%u\n", mesh_id.slot, mesh_id.generation);

    // ── Material ──────────────────────────────────────────────────────────█
    printf("[helio] Creating material...\n");
    HelioGpuMaterial mat{};
    mat.base_color[0]  = 0.8f;
    mat.base_color[1]  = 0.2f;
    mat.base_color[2]  = 0.2f;
    mat.base_color[3]  = 1.0f;
    mat.roughness_metallic[0] = 0.5f;
    mat.roughness_metallic[1] = 0.3f;

    HelioHandle material_id = helio_scene_insert_material(scene, &mat);
    printf("[helio] material: slot=%u gen=%u\n", material_id.slot, material_id.generation);

    // ── Object ────────────────────────────────────────────────────────────█
    printf("[helio] Creating object...\n");
    HelioObjectDescriptor obj_desc{};
    obj_desc.mesh     = mesh_id;
    obj_desc.material = material_id;
    obj_desc.transform = identity();
    obj_desc.bounds[3] = 2.0f;  // bounding sphere radius

    HelioResult result = helio_scene_insert_object(scene, &obj_desc);
    printf("[helio] object inserted: %d\n", result.success);
    if (!result.success && result.error_message) {
        printf("  error: %s\n", result.error_message);
        helio_free_error_string(result.error_message);
    }

    helio_scene_flush(scene);

    // ── Light ─────────────────────────────────────────────────────────────█
    printf("[helio] Creating light...\n");
    HelioGpuLight light{};
    light.color_intensity[0] = 1.0f;
    light.color_intensity[1] = 1.0f;
    light.color_intensity[2] = 1.0f;
    light.color_intensity[3] = 10.0f;
    light.position_range[0] = 0;
    light.position_range[1] = 5;
    light.position_range[2] = 0;
    light.position_range[3] = 20;
    light.direction_outer[1] = -1;
    light.light_type = 2; // point

    HelioHandle light_id = helio_scene_insert_light(scene, &light);
    printf("[helio] light: slot=%u gen=%u\n", light_id.slot, light_id.generation);

    // ── Renderer ──────────────────────────────────────────────────────────█
    // Scene ownership transfers to the renderer — do NOT use `scene` after this.
    printf("[helio] Creating renderer...\n");
    HelioRendererConfig cfg{};
    cfg.width            = (uint32_t)w;
    cfg.height           = (uint32_t)h;
    cfg.render_scale     = 1.0f;
    cfg.shadow_quality   = 1;
    cfg.shadow_atlas_size = 1024;
    cfg.shadow_face_capacity = 16;
    cfg.gi_rc_radius     = 80.0f;
    cfg.gi_fade_margin   = 20.0f;

    uint32_t surface_format = 1; // Rgba8UnormSrgb

    HelioRendererPtr renderer = helio_renderer_new(
        device_ptr, queue_ptr, surface_format, &cfg,
        scene, debug_cam, cull_stats
    );
    // `scene`, `debug_cam`, `cull_stats` are now owned by the renderer.
    printf("[helio] renderer=%p\n", (void*)renderer);

    // Get the scene pointer back from the renderer.
    HelioScenePtr rs = helio_renderer_scene(renderer);
    printf("[helio] renderer scene=%p\n", (void*)rs);

    // ── Editor & Picker (non-GPU smoke test) ──────────────────────────────█
    printf("[helio] Testing editor...\n");
    HelioEditorPtr editor = helio_editor_new();
    printf("[helio] editor: gizmo_mode=%u dragged=%d hovered_axis=%d\n",
           helio_editor_gizmo_mode(editor),
           helio_editor_is_dragging(editor),
           helio_editor_hovered_axis(editor));

    HelioSceneActorId sid = helio_editor_selected(editor);
    printf("[helio] editor selected type=%d\n", sid.actor_type);

    helio_editor_select(editor, sid);
    helio_editor_set_gizmo_mode(editor, 1);
    printf("[helio] editor gizmo_mode after set: %u\n", helio_editor_gizmo_mode(editor));

    printf("[helio] Testing picker...\n");
    HelioPickerPtr picker = helio_picker_new();

    helio_picker_register_mesh(
        picker, mesh_id,
        (const uint8_t*)verts.data(), (uint32_t)verts.size(),
        (const uint8_t*)idx.data(), (uint32_t)idx.size()
    );
    helio_picker_rebuild_instances(picker, rs);

    float origin[3] = { 0, 0, 10 };
    float dir[3]    = { 0, 0, -1 };
    HelioPickHit hit{};
    bool picked = helio_picker_cast_ray(picker, rs, origin, dir, &hit);
    printf("[helio] picker ray hit: %d\n", picked);
    if (picked) {
        printf("  t=%f pos=(%f,%f,%f)\n", hit.t, hit.position.x, hit.position.y, hit.position.z);
    }

    // MeshUpload helper
    printf("[helio] Testing MeshUpload helper...\n");
    HelioMeshUploadPtr mu = helio_mesh_upload_create(
        (const uint8_t*)verts.data(), (uint32_t)verts.size(),
        (const uint8_t*)idx.data(), (uint32_t)idx.size()
    );
    helio_mesh_upload_free(mu);
    printf("[helio] MeshUpload OK\n");

    // ── Render loop ───────────────────────────────────────────────────────█
    printf("[helio] Entering render loop...\n");
    int frame_count = 0;
    double last_time = glfwGetTime();
    bool running = true;

    while (running && !glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Resize
        int fb_w, fb_h;
        glfwGetFramebufferSize(window, &fb_w, &fb_h);
        if (fb_w != w || fb_h != h) {
            w = fb_w;
            h = fb_h;
            helio_renderer_output_size(renderer, (uint32_t*)&fb_w, (uint32_t*)&fb_h);
        }

        // Animate camera
        float angle = (float)glfwGetTime() * 0.3f;
        float cx = 5.0f * sinf(angle);
        float cz = 5.0f * cosf(angle);
        cam = helio_camera_perspective_look_at(
            cx, 3, cz,
            0, 0, 0,
            1.0472f, (float)w / (float)h,
            0.1f, 100.0f
        );
        helio_scene_update_camera(rs, &cam);

        // Render
        double t0 = glfwGetTime();
        void* target_view = bootstrap_current_texture_view();
        double t1 = glfwGetTime();
        double ms_acq = (t1 - t0) * 1000.0;
        if (target_view) {
            HelioResult r = helio_renderer_render(renderer, &cam, target_view);
            double t2 = glfwGetTime();
            if (!r.success && r.error_message) {
                fprintf(stderr, "Render error: %s\n", r.error_message);
                helio_free_error_string(r.error_message);
            }
            bootstrap_present();
            double t3 = glfwGetTime();
            double ms_render = (t2 - t1) * 1000.0;
            double ms_present = (t3 - t2) * 1000.0;
            printf("[perf] acq=%.1fms render=%.1fms present=%.1fms total=%.1fms\n",
                ms_acq, ms_render, ms_present, (t3 - t0) * 1000.0);
        } else {
            printf("[perf] ACQUIRE FAILED (outdated/lost) after %.0fms\n", ms_acq);
        }

        // FPS
        frame_count++;
        double now = glfwGetTime();
        if (now - last_time >= 1.0) {
            printf("[helio] FPS: %d\n", frame_count);
            frame_count = 0;
            last_time = now;
        }

        bootstrap_poll(false);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE))
            running = false;
    }

    // ── Cleanup ───────────────────────────────────────────────────────────█
    printf("[helio] Shutting down...\n");
    helio_editor_destroy(editor);
    helio_picker_destroy(picker);
    helio_renderer_destroy(renderer);
    bootstrap_shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    printf("[helio] Done.\n");
    return 0;
}
