/// Helio ImGui Test App — just clear + present, no helio rendering
#include <cstdio>
#include <cstdlib>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#undef near
#undef far
#include "bootstrap.h"

static void fail(const char* msg) { fprintf(stderr, "FATAL: %s\n", msg); exit(1); }

int main() {
    glfwSetErrorCallback([](int e, const char* d) { fprintf(stderr, "GLFW %d: %s\n", e, d); });
    if (!glfwInit()) fail("glfwInit");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Helio Test", NULL, NULL);
    if (!window) fail("glfwCreateWindow");

    void *dev=0,*q=0,*dc=0,*cs=0;
    if (!bootstrap_init(1280,720,&dev,&q,&dc,&cs)) fail("bootstrap_init");
    HWND h = glfwGetWin32Window(window);
    if (!bootstrap_create_surface((void*)GetModuleHandleW(0),(void*)h)) fail("surface");

    int fc=0; double lt=glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        bootstrap_render_frame(nullptr,nullptr);
        if (++fc, (glfwGetTime()-lt)>=1.0) { printf("[helio] FPS: %d\n",fc); fc=0; lt=glfwGetTime(); }
    }
    bootstrap_shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
