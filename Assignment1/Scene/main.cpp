/**
 * @file    main.cpp
 * @brief   Entry point for Desktop (GLFW) and Web (Emscripten) builds.
 *          Only talks to Renderer::Instance() - no shape is referenced here.
 * @author Santhosh Ravikumar
 * @date    2026
 */
#include "Platform.h"
#include "Renderer.h"

// ============================================================================
// Web / Emscripten (GLFW backend)
// ============================================================================
#ifdef PLATFORM_EMSCRIPTEN

static GLFWwindow* g_window = nullptr;

static void OnFramebufferResize(GLFWwindow* /*win*/, int w, int h)
{
    Renderer::Instance().resize(w, h);
}

static void MainLoop()
{
    if (glfwWindowShouldClose(g_window)) {
        emscripten_cancel_main_loop();
        glfwDestroyWindow(g_window);
        glfwTerminate();
        return;
    }

    glfwPollEvents();
    Renderer::Instance().render();
    glfwSwapBuffers(g_window);
}

int main()
{
    LOGI("AndroidProgrammingQuiz - Web (Emscripten/GLFW)");

    if (!glfwInit()) {
        LOGE("glfwInit failed");
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    g_window = glfwCreateWindow(800, 600, "AndroidProgrammingQuiz", nullptr, nullptr);
    if (!g_window) {
        LOGE("glfwCreateWindow failed");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(g_window);
    glfwSetFramebufferSizeCallback(g_window, OnFramebufferResize);

    if (!Renderer::Instance().initializeRenderer()) {
        LOGE("Renderer initialization failed");
        glfwDestroyWindow(g_window);
        glfwTerminate();
        return -1;
    }

    int w, h;
    glfwGetFramebufferSize(g_window, &w, &h);
    Renderer::Instance().resize(w, h);

    emscripten_set_main_loop(MainLoop, 0, 1);
    return 0;
}

// ============================================================================
// Desktop / Windows (GLFW + GLEW)
// ============================================================================
#elif defined(PLATFORM_WINDOWS)

#include <iostream>

static void OnFramebufferResize(GLFWwindow* /*win*/, int w, int h)
{
    Renderer::Instance().resize(w, h);
}

int main()
{
    std::cout << "AndroidProgrammingQuiz - Desktop (GLFW)\n";
    std::cout << "Press ESC to quit.\n";

    if (!glfwInit()) {
        std::cerr << "glfwInit failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "AndroidProgrammingQuiz", nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "glewInit failed\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, OnFramebufferResize);

    if (!Renderer::Instance().initializeRenderer()) {
        std::cerr << "Renderer initialization failed\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    Renderer::Instance().resize(w, h);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        Renderer::Instance().render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

#endif // PLATFORM_WINDOWS
