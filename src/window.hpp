#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void createWindow();

struct window {
    GLFWwindow* window;
    const int width = 800;
    const int height = 600;
    const char* title = "Vulkan Win";
};

extern window Window;