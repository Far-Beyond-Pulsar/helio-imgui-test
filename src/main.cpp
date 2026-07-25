/// Helio ImGui Test App
///
/// Exercises the C FFI bindings (helio_ffi) by creating a window, initialising
/// wgpu via the built-in bootstrap helper, and rendering Dear ImGui.

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

// ── Main ───────────────────────────────────────────────────────────────────────

int main() {
    int w = 1280, h = 720;

    // ── Window ────────────────────────────────────────────────────────────█
    glfwSetErrorCallback([](int e, const char* d) { fprintf(stderr, "GLFW %d: %s\n", e, d); });
    if (!glfwInit()) fail("glfwInit");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(w, h, "Helio ImGui Test", NULL, NULL);
    if (!window) fail("glfwCreateWindow");

    // ── Bootstrap wgpu ────────────────────────────────────────────────────█
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

    // ── FPS counter ───────────────────────────────────────────────────────█
    printf("[helio] Entering render loop...\n");
    int frame_count = 0;
    double last_time = glfwGetTime();
    bool running = true;

    while (running && !glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Submit a render frame (acquire, render nothing useful, present)
        bootstrap_render_frame(nullptr, nullptr);

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
    bootstrap_shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    printf("[helio] Done.\n");
    return 0;
}
