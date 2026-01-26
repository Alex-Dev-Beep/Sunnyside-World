#include "surface.hpp"
#include "instance.hpp"
#include "window.hpp"

#include <SDL3/SDL_vulkan.h>
#include <stdexcept>
#include <iostream>

surface Surface;

void createSurface() {
    std::cout << "Creating Vulkan surface..." << std::endl;

    if (!SDL_Vulkan_CreateSurface(
        Window.window,
        Instance.instance,
        nullptr,
        &Surface.surface
    )) {
        throw std::runtime_error(SDL_GetError());
    }

    std::cout << "Successfully created Vulkan surface!" << std::endl;
}
