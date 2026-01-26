#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

void createSurface();

struct surface {
    public:
    VkSurfaceKHR surface = VK_NULL_HANDLE;
};

extern surface Surface;