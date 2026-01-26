#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

void createWindow();

struct window {
    SDL_Window* window = nullptr;
    const int width = 800;
    const int height = 600;
    const char* title = "Vulkan Window";
};

extern window Window;