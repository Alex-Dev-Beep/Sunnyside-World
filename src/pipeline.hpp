#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>

struct pipeline {
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
};

void createGraphicsPipeline();
void createRenderPass();
void createFramebuffers(
    std::vector<VkFramebuffer>& swapChainFramebuffers,
    const std::vector<VkImageView>& swapChainImageViews,
    VkRenderPass renderPass,
    VkExtent2D swapChainExtent,
    VkDevice device,
    VkImageView depthImageView
);

extern pipeline Pipeline;