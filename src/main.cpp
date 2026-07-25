#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "bootstrap.h"

static void fail(const char* msg) { fprintf(stderr, "FATAL: %s\n", msg); exit(1); }

// ── Viewport texture ──────────────────────────────────────────────────────────
static GLuint g_viewport_tex = 0;
static void ensure_viewport_tex(int w, int h) {
    if (!g_viewport_tex) glGenTextures(1, &g_viewport_tex);
    glBindTexture(GL_TEXTURE_2D, g_viewport_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int main() {
    if (!glfwInit()) fail("glfwInit");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* w = glfwCreateWindow(1600, 900, "Helio Editor", NULL, NULL);
    if (!w) fail("glfwCreateWindow");
    glfwMakeContextCurrent(w);
    glfwSwapInterval(1);

    // ── Helio ──────────────────────────────────────────────────────────────
    void *dev=0,*q=0,*dc=0,*cs=0;
    if (!bootstrap_init(1280,720,&dev,&q,&dc,&cs)) fail("bootstrap_init");
    HWND hw = glfwGetWin32Window(w);
    if (!bootstrap_create_surface((void*)GetModuleHandleW(0),(void*)hw)) fail("surface");

    printf("[helio] Creating scene...\n");
    HelioScenePtr scene = helio_scene_new(dev, q);

    // Camera
    HelioCameraDesc cam = helio_camera_perspective_look_at(5,3,5, 0,0,0, 1.0472f, 16/9.f, 0.1f, 100.f);
    helio_scene_update_camera(scene, &cam);

    // Mesh (triangle)
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

    HelioObjectDescriptor od{}; od.mesh=mesh; od.material=material; od.transform = HelioMat4();
    od.transform.data[0][0]=1; od.transform.data[1][1]=1; od.transform.data[2][2]=1; od.transform.data[3][3]=1;
    od.bounds[3]=2;
    helio_scene_insert_object(scene, &od);
    helio_scene_flush(scene);

    // Light
    HelioGpuLight lt{}; lt.color_intensity[0]=1; lt.color_intensity[1]=1; lt.color_intensity[2]=1; lt.color_intensity[3]=10;
    lt.position_range[0]=0; lt.position_range[1]=5; lt.position_range[2]=0; lt.position_range[3]=20;
    lt.light_type=2;
    helio_scene_insert_light(scene, &lt);

    // Renderer
    HelioRendererConfig cfg{}; cfg.width=1280; cfg.height=720; cfg.render_scale=1;
    HelioRendererPtr ren = helio_renderer_new(dev,q,1,&cfg,scene,dc,cs);
    HelioScenePtr rs = helio_renderer_scene(ren);
    printf("[helio] Ready.\n");

    // ── ImGui ──────────────────────────────────────────────────────────────
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(w, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    bool show_demo = false;
    int vp_w = 1280, vp_h = 720;

    while (!glfwWindowShouldClose(w)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Dockspace
        ImGuiWindowFlags df = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        ImGui::Begin("DockSpace", nullptr, df);
        ImGui::PopStyleVar();
        ImGui::DockSpace(ImGui::GetID("DockSpace"));
        ImGui::End();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) { ImGui::EndMenu(); }
            if (ImGui::BeginMenu("View")) { ImGui::MenuItem("Demo", 0, &show_demo); ImGui::EndMenu(); }
            ImGui::EndMainMenuBar();
        }

        ImGui::Begin("Scene");
        ImGui::Text("Root"); ImGui::Indent();
        ImGui::BulletText("Camera"); ImGui::BulletText("Light"); ImGui::BulletText("Triangle");
        ImGui::Unindent(); ImGui::End();

        ImGui::Begin("Properties");
        float pos[3]={0},rot[3]={0},scl[3]={1,1,1};
        ImGui::DragFloat3("Position",pos,0.1f);
        ImGui::DragFloat3("Rotation",rot,1);
        ImGui::DragFloat3("Scale",scl,0.1f);
        float color[4]={0.8f,0.2f,0.2f,1};
        ImGui::ColorEdit4("Color",color);
        ImGui::Text("Triangles: 1");
        ImGui::End();

        // Viewport
        ImGui::Begin("Viewport");
        ImVec2 avail = ImGui::GetContentRegionAvail();
        vp_w = (int)avail.x; vp_h = (int)avail.y;
        if (vp_w < 1) vp_w = 1; if (vp_h < 1) vp_h = 1;

        // Update camera animation
        float angle = (float)glfwGetTime() * 0.3f;
        cam = helio_camera_perspective_look_at(5*sinf(angle),3,5*cosf(angle), 0,0,0, 1.0472f, (float)vp_w/vp_h, 0.1f, 100.f);
        helio_scene_update_camera(rs, &cam);

        // Render helio to CPU buffer
        std::vector<uint8_t> pixels(vp_w * vp_h * 4);
        bootstrap_render_viewport(ren, &cam, vp_w, vp_h, pixels.data());

        // Upload to OpenGL texture
        ensure_viewport_tex(vp_w, vp_h);
        glBindTexture(GL_TEXTURE_2D, g_viewport_tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vp_w, vp_h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

        ImGui::Image((ImTextureID)(intptr_t)g_viewport_tex, avail);
        ImGui::End();

        if (show_demo) ImGui::ShowDemoWindow(&show_demo);

        ImGui::Render();
        int fb_w, fb_h;
        glfwGetFramebufferSize(w, &fb_w, &fb_h);
        glViewport(0, 0, fb_w, fb_h);
        glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(w);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    bootstrap_shutdown();
    glfwDestroyWindow(w);
    glfwTerminate();
    return 0;
}
