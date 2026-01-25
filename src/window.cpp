#include "window.hpp"

#include <stdexcept>
#include <iostream>

window Window;

void createWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    Window.window = glfwCreateWindow(Window.width, Window.height, Window.title, nullptr, nullptr);

    if (!Window.window) {
        throw std::runtime_error("Failed to create GLFW window!");
    }

    std::cout << "Created GLFW window!" << std::endl;
}
