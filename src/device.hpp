#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "vertex.hpp"

#include <optional>
#include <vector>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct device {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
};

struct queues {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

void pickPhysicalDevice();
bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
void createLogicalDevice();
void createCommandPool(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool& commandPool, VkDevice device);
void createCommandBuffer(VkCommandPool commandPool, VkDevice device, int MAX_FRAMES_IN_FLIGHT, std::vector<VkCommandBuffer>& commandBuffers);
void recordCommandBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex,
    VkRenderPass renderPass,
    const std::vector<VkFramebuffer>& framebuffers,
    VkExtent2D extent,
    VkPipeline pipeline,
    VkBuffer vertexBuffer,
    VkBuffer indexBuffer,
    const std::vector<uint16_t>& indices,
    VkPipelineLayout pipelineLayout,
    const std::vector<VkDescriptorSet>& descriptorSets
);

extern device Device;
extern queues Queues;