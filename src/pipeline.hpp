#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>

void createGraphicsPipeline(VkDevice device, VkExtent2D swapChainExtent, VkPipelineLayout& pipelineLayout, VkRenderPass renderPass, VkPipeline& graphicsPipeline, VkDescriptorSetLayout descriptorSetLayout);
void createRenderPass(VkFormat swapChainImageFormat, VkRenderPass& renderPass, VkDevice device, VkPhysicalDevice physicalDevice);
void createFramebuffers(
    std::vector<VkFramebuffer>& swapChainFramebuffers,
    const std::vector<VkImageView>& swapChainImageViews,
    VkRenderPass renderPass,
    VkExtent2D swapChainExtent,
    VkDevice device,
    VkImageView depthImageView
);