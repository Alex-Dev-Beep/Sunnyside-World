#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void createInstance(VkInstance& instance);
bool checkValidationLayerSupport();

void setupDebugMessenger(VkInstance instance);

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* messenger
);

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* allocator
);

void cleanupInstance(VkInstance instance);
