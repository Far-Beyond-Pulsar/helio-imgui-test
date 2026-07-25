#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool bootstrap_init(uint32_t width, uint32_t height, void** out_device, void** out_queue, void** out_debug_cam, void** out_cull_stats);
bool bootstrap_create_surface(void* hinstance, void* hwnd);
uint32_t bootstrap_get_format(void);
void* bootstrap_current_texture_view(void);
void bootstrap_present(void);
void bootstrap_poll(bool wait);
bool bootstrap_render_frame(void* renderer, const void* camera);

// Viewport: render helio scene to CPU buffer for ImGui display
bool bootstrap_render_viewport(void* renderer, const void* camera, uint32_t width, uint32_t height, uint8_t* out_rgba);

void bootstrap_shutdown(void);

#ifdef __cplusplus
}
#endif
