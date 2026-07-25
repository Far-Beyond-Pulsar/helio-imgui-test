/// Simple helio render test — renders to the swapchain, no ImGui viewport
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#undef near
#undef far
#include "bootstrap.h"
#include <helio/helio_capi.h>

static void fail(const char* msg) { fprintf(stderr, "FATAL: %s\n", msg); exit(1); }

int main() {
    if (!glfwInit()) fail("glfwInit");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* w = glfwCreateWindow(1280, 720, "Helio Test", NULL, NULL);
    if (!w) fail("glfwCreateWindow");

    void *dev=0,*q=0,*dc=0,*cs=0;
    if (!bootstrap_init(1280,720,&dev,&q,&dc,&cs)) fail("bootstrap_init");
    HWND hw = glfwGetWin32Window(w);
    if (!bootstrap_create_surface((void*)GetModuleHandleW(0),(void*)hw)) fail("surface");

    // Scene
    HelioScenePtr scene = helio_scene_new(dev, q);

    // Camera
    auto cam = helio_camera_perspective_look_at(5,3,5,0,0,0,1.0472f,1280/720.f,0.1f,100.f);
    helio_scene_update_camera(scene, &cam);

    // Triangle mesh
    std::vector<HelioPackedVertex> verts = {
        {{0,1,0},0,{0.5f,1},{0,0},0x7F0000,0x7F0000},
        {{-1,-1,0},0,{0,0},{0,0},0x007F00,0x007F00},
        {{1,-1,0},0,{1,0},{0,0},0x00007F,0x00007F},
    };
    std::vector<uint32_t> idx = {0,1,2};
    HelioMeshUpload mu; mu.vertices=verts.data(); mu.vertex_count=3; mu.indices=idx.data(); mu.index_count=3;
    HelioHandle mesh = helio_scene_insert_mesh(scene, &mu);

    HelioGpuMaterial mat{}; mat.base_color[0]=0.8f; mat.base_color[1]=0.2f; mat.base_color[2]=0.2f; mat.base_color[3]=1;
    HelioHandle material = helio_scene_insert_material(scene, &mat);

    HelioObjectDescriptor od{}; od.mesh=mesh; od.material=material; od.bounds[3]=2;
    od.transform.data[0][0]=1; od.transform.data[1][1]=1; od.transform.data[2][2]=1; od.transform.data[3][3]=1;
    helio_scene_insert_object(scene, &od);
    helio_scene_flush(scene);

    HelioGpuLight lt{}; lt.color_intensity[0]=1; lt.color_intensity[1]=1; lt.color_intensity[2]=1; lt.color_intensity[3]=10;
    lt.position_range[0]=0; lt.position_range[1]=5; lt.position_range[2]=0; lt.position_range[3]=20; lt.light_type=2;
    helio_scene_insert_light(scene, &lt);

    HelioRendererConfig cfg{}; cfg.width=1280; cfg.height=720; cfg.render_scale=1;
    HelioRendererPtr ren = helio_renderer_new(dev,q,1,&cfg,scene,dc,cs);
    HelioScenePtr rs = helio_renderer_scene(ren);

    int fc=0; double lt=glfwGetTime();
    while (!glfwWindowShouldClose(w)) {
        glfwPollEvents();
        float a = (float)glfwGetTime() * 0.3f;
        cam = helio_camera_perspective_look_at(5*sinf(a),3,5*cosf(a),0,0,0,1.0472f,1280/720.f,0.1f,100.f);
        helio_scene_update_camera(rs, &cam);
        bootstrap_render_frame(ren, &cam);
        if (++fc, (glfwGetTime()-lt)>=1.0) { printf("[helio] FPS: %d\n",fc); fc=0; lt=glfwGetTime(); }
    }
    helio_renderer_destroy(ren);
    bootstrap_shutdown();
    glfwDestroyWindow(w);
    glfwTerminate();
    return 0;
}
