#include "window.hpp"

#include <stdexcept>
#include <iostream>

window Window;

void createWindow() {
    Window.window = SDL_CreateWindow(
        Window.title,
        Window.width,
        Window.height,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    if (!Window.window) {
        throw std::runtime_error(SDL_GetError());
    }

    std::cout << "Created SDL window!" << std::endl;
}
