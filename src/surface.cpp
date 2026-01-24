#include "surface.hpp"

#include <stdexcept>
#include <iostream>

void createSurface(VkInstance instance, VkSurfaceKHR& surface, GLFWwindow*& window) {
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = glfwGetWin32Window(window);
    createInfo.hinstance = GetModuleHandle(nullptr);

    if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    } else {
        std::cout << "Succesfully created window surface!" << std::endl;
    }
}