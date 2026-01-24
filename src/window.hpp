#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void createWindow(int width, int height, const char* title, GLFWwindow*& window);
