#include "window.hpp"

#include <stdexcept>
#include <iostream>

void createWindow(int width, int height, const char* title, GLFWwindow*& window) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if (!window) {
        throw std::runtime_error("Failed to create GLFW window!");
    }

    std::cout << "Created GLFW window!" << std::endl;
}
