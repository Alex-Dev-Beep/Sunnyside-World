#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

void pickPhysicalDevice(VkInstance instance, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface);
bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
void createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice& device, VkQueue& graphicsQueue, VkQueue& presentQueue);
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