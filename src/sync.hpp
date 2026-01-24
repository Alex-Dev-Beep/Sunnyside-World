#define GLFW_INCLUDE_VULKAN
#include <GLFW//glfw3.h>

#include <vector>

void createSyncObjects(
    VkDevice device,
    std::vector<VkSemaphore>& imageAvailableSemaphores,
    std::vector<VkSemaphore>& renderFinishedSemaphores,
    std::vector<VkFence>& inFlightFences,
    int MAX_FRAMES_IN_FLIGHT
) ;