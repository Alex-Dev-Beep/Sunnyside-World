#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

void createSurface();

struct surface {
    public:
    VkSurfaceKHR surface = VK_NULL_HANDLE;
};

extern surface Surface;