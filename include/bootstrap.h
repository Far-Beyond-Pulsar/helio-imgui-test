#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Initialise wgpu and return pointers to device, queue, and auxiliary buffers.
/// Call once before any helio-ffi functions.
bool bootstrap_init(
    uint32_t width, uint32_t height,
    void** out_device,
    void** out_queue,
    void** out_debug_cam,
    void** out_cull_stats
);

/// Create a wgpu surface from a Win32 HWND.  Call after bootstrap_init().
bool bootstrap_create_surface(void* hinstance, void* hwnd);

/// Get the current swapchain texture view.  Returns a pointer suitable
/// for passing as HelioTextureViewPtr to helio_renderer_render().
void* bootstrap_current_texture_view(void);

/// Present the current frame to the surface.
void bootstrap_present(void);

/// Poll wgpu device (process async callbacks).
void bootstrap_poll(bool wait);

/// Destroy all resources.  Call at exit.
void bootstrap_shutdown(void);

#ifdef __cplusplus
}
#endif
