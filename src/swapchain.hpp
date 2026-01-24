#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vertex.hpp"

#include <vector>

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

void createSwapChain(VkPhysicalDevice physicalDevice, 
    VkSurfaceKHR surface, 
    GLFWwindow* window, 
    VkSwapchainKHR& swapChain, 
    VkDevice device, 
    std::vector<VkImage>& swapChainImages, 
    VkFormat& swapChainImageFormat, 
    VkExtent2D& swapChainExtent
);

void createImageViews(std::vector<VkImageView>& swapChainImageViews, std::vector<VkImage>& swapChainImages, VkFormat swapChainImageFormat, VkDevice device);
void cleanupSwapChain(VkDevice& device, VkRenderPass& renderPass, std::vector<VkFramebuffer>& swapChainFramebuffers, VkCommandPool& commandPool, std::vector<VkCommandBuffer>& commandBuffers, std::vector<VkImageView>& swapChainImageViews, VkSwapchainKHR& swapChain);
void recreateSwapChain(GLFWwindow* window, VkDevice& device, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkSwapchainKHR& swapChain, int MAX_FRAMES_IN_FLIGHT, VkRenderPass& renderPass, VkCommandPool& commandPool, std::vector<VkCommandBuffer>& commandBuffers, VkExtent2D& swapChainExtent, std::vector<VkImage>& swapChainImages, VkFormat& swapChainImageFormat, std::vector<VkImageView>& swapChainImageViews, std::vector<VkFramebuffer>& swapChainFramebuffers, VkPipeline& graphicsPipeline, VkPipelineLayout& pipelineLayout, VkBuffer vertexBuffer, std::vector<Vertex> vertices, VkBuffer indexBuffer, const std::vector<uint16_t> indices, VkDescriptorSetLayout descriptorSetLayout, std::vector<VkDescriptorSet> descriptorSets, uint32_t currentFrame, VkQueue graphicsQueue, VkImageView depthImageView, VkDeviceMemory depthImageMemory, VkImage depthImage);