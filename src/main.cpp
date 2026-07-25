/// Game Engine-style ImGui test app
#include <cstdio>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

static void fail(const char* msg) { fprintf(stderr, "FATAL: %s\n", msg); exit(1); }

int main() {
    if (!glfwInit()) fail("glfwInit");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* w = glfwCreateWindow(1600, 900, "Engine Editor", NULL, NULL);
    if (!w) fail("glfwCreateWindow");
    glfwMakeContextCurrent(w);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(w, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    bool show_demo = true;
    while (!glfwWindowShouldClose(w)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) { ImGui::MenuItem("New"); ImGui::MenuItem("Open"); ImGui::MenuItem("Save"); ImGui::EndMenu(); }
            if (ImGui::BeginMenu("Edit")) { ImGui::MenuItem("Undo"); ImGui::MenuItem("Redo"); ImGui::EndMenu(); }
            if (ImGui::BeginMenu("View")) { ImGui::MenuItem("Demo", 0, &show_demo); ImGui::EndMenu(); }
            ImGui::EndMainMenuBar();
        }

        ImGui::Begin("Scene Hierarchy");
        ImGui::Text("Root");
        ImGui::Indent();
        ImGui::BulletText("Camera");
        ImGui::BulletText("Directional Light");
        ImGui::BulletText("Cube");
        ImGui::Unindent();
        ImGui::End();

        ImGui::Begin("Properties");
        float pos[3] = {0,0,0}, rot[3] = {0,0,0}, scl[3] = {1,1,1};
        ImGui::DragFloat3("Position", pos, 0.1f);
        ImGui::DragFloat3("Rotation", rot, 1.0f);
        ImGui::DragFloat3("Scale",   scl, 0.1f);
        ImGui::Separator();
        ImGui::Text("Mesh: Cube (12 triangles)");
        ImGui::Separator();
        float color[4] = {0.8f, 0.2f, 0.2f, 1.0f};
        ImGui::ColorEdit4("Color", color);
        ImGui::End();

        ImGui::Begin("Viewport");
        ImVec2 vp = ImGui::GetContentRegionAvail();
        ImGui::Image((ImTextureID)(intptr_t)0, vp, ImVec2(0,1), ImVec2(1,0));
        auto dl = ImGui::GetWindowDrawList();
        ImVec2 c = ImGui::GetWindowPos() + ImGui::GetWindowSize() * 0.5f;
        dl->AddLine(ImVec2(c.x-20,c.y), ImVec2(c.x+20,c.y), IM_COL32(255,255,255,80));
        dl->AddLine(ImVec2(c.x,c.y-20), ImVec2(c.x,c.y+20), IM_COL32(255,255,255,80));
        ImGui::End();

        if (show_demo) ImGui::ShowDemoWindow(&show_demo);

        ImGui::Render();
        int ww, wh;
        glfwGetFramebufferSize(w, &ww, &wh);
        glViewport(0, 0, ww, wh);
        glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(w);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(w);
    glfwTerminate();
    return 0;
}
