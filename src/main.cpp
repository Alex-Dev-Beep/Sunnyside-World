#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <stdexcept>
#include <iostream>
#include <vector>

#include "window.hpp"
#include "instance.hpp"
#include "surface.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"
#include "sync.hpp"
#include "vertex.hpp"
#include "uniform.hpp"
#include "image.hpp"

// document everything

bool framebufferResized = false;
uint32_t currentFrame = 0;
uint32_t imageIndex;

std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 0.3f}, {0.0f, 1.0f}},
    {{ 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 0.3f}, {1.0f, 1.0f}},
    {{ 0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 0.3f}, {1.0f, 0.0f}},
    {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 0.3f}, {0.0f, 0.0f}}
};


std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

void framebufferResizeCallback(
SDL_Window* window, int width, int height) {
    framebufferResized = true;
}

void drawFrame(
    VkDevice& device,
    VkSwapchainKHR& swapChain,
    VkRenderPass& renderPass,
    std::vector<VkFramebuffer>& swapChainFramebuffers,
    VkExtent2D& swapChainExtent,
    VkPipeline& graphicsPipeline,
    VkQueue& graphicsQueue,
    VkQueue& presentQueue,
    std::vector<VkFence>& inFlightFences,
    std::vector<VkSemaphore>& renderFinishedSemaphores,
    std::vector<VkSemaphore>& imageAvailableSemaphores,
    std::vector<VkCommandBuffer>& commandBuffers,
    int MAX_FRAMES_IN_FLIGHT,
    VkSurfaceKHR& surface,
    SDL_Window* window,
    VkPhysicalDevice& physicalDevice,
    VkCommandPool& commandPool,
    VkFormat& swapChainImageFormat,
    std::vector<VkImage>& swapChainImages,
    std::vector<VkImageView>& swapChainImageViews,
    VkPipelineLayout& pipelineLayout,
    VkBuffer vertexBuffer,
    const std::vector<Vertex>& vertices,
    VkBuffer indexBuffer,
    VkDescriptorSetLayout descriptorSetLayout,
    std::vector<void*>& uniformBuffersMapped,
    std::vector<VkDescriptorSet>& descriptorSets,
    VkImage& depthImage,
    VkDeviceMemory& depthImageMemory,
    VkImageView& depthImageView,
    std::vector<VkFence>& imagesInFlight
)
 {
    vkWaitForFences(
        device,
        1,
        &inFlightFences[currentFrame],
        VK_TRUE,
        UINT64_MAX
    );

    VkResult result = vkAcquireNextImageKHR(
        device,
        swapChain,
        UINT64_MAX,
        imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(window, device, physicalDevice, surface, swapChain, MAX_FRAMES_IN_FLIGHT, renderPass, commandPool, commandBuffers, swapChainExtent, swapChainImages, swapChainImageFormat, swapChainImageViews, swapChainFramebuffers, graphicsPipeline, pipelineLayout, vertexBuffer, vertices, indexBuffer, indices, descriptorSetLayout, descriptorSets, currentFrame, graphicsQueue, depthImageView, depthImageMemory, depthImage);
        imagesInFlight.clear();
        imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    updateUniformBuffer(currentFrame, uniformBuffersMapped, swapChainExtent);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];


    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(
        graphicsQueue,
        1,
        &submitInfo,
        inFlightFences[currentFrame]
    ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR ||
        framebufferResized) {

        framebufferResized = false;
        recreateSwapChain(window, device, physicalDevice, surface, swapChain, MAX_FRAMES_IN_FLIGHT, renderPass, commandPool, commandBuffers, swapChainExtent, swapChainImages, swapChainImageFormat, swapChainImageViews, swapChainFramebuffers, graphicsPipeline, pipelineLayout, vertexBuffer, vertices, indexBuffer, indices, descriptorSetLayout, descriptorSets, currentFrame, graphicsQueue, depthImageView, depthImageMemory, depthImage);
        imagesInFlight.clear();
        imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
        return;
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

int main() {
    std::cout << "hey" << std::endl;
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        throw std::runtime_error("Failed to init SDL!");
    } else {
        std::cout << "Initialized SDL correctly!" << std::endl;
    }
    std::vector<VkFence> imagesInFlight;
    std::vector<VkImageView> swapChainImageViews;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;    
    std::vector<VkDescriptorSet> descriptorSets;    
    const int MAX_FRAMES_IN_FLIGHT = 2; 
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkDescriptorPool descriptorPool;    
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    
    createWindow();
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    imagesInFlight.resize(SwapChain.swapChainImages.size(), VK_NULL_HANDLE);
    createImageViews(swapChainImageViews, SwapChain.swapChainImages, SwapChain.swapChainImageFormat, Device.device);
    createRenderPass(SwapChain.swapChainImageFormat, renderPass, Device.device, Device.physicalDevice);
    createDescriptorSetLayout(Device.device, descriptorSetLayout);
    createGraphicsPipeline(Device.device, SwapChain.swapChainExtent, pipelineLayout, renderPass, graphicsPipeline, descriptorSetLayout);
    createCommandPool(Device.physicalDevice, Surface.surface, commandPool, Device.device);
    createDepthResources(SwapChain.swapChainExtent, depthImageView, depthImage, depthImageMemory, Device.device, Device.physicalDevice, commandPool, Queues.graphicsQueue);
    createFramebuffers(swapChainFramebuffers, swapChainImageViews, renderPass, SwapChain.swapChainExtent, Device.device, depthImageView);
    createTextureImage(Device.physicalDevice, Device.device, textureImage, textureImageMemory, Queues.graphicsQueue, commandPool);
    createTextureImageView(Device.device, textureImage, textureImageView);
    createTextureSampler(Device.physicalDevice, textureSampler, Device.device);
    createVertexBuffer(Device.physicalDevice, Device.device, commandPool, Queues.graphicsQueue, vertexBuffer, vertexBufferMemory, vertices);
    createUniformBuffers(MAX_FRAMES_IN_FLIGHT, Device.device, uniformBuffers, uniformBuffersMemory, uniformBuffersMapped, Device.physicalDevice);
    createDescriptorPool(MAX_FRAMES_IN_FLIGHT, descriptorPool, Device.device);
    createDescriptorSets(MAX_FRAMES_IN_FLIGHT, descriptorPool, descriptorSetLayout, descriptorSets, Device.device, uniformBuffers, textureSampler, textureImageView);
    createIndexBuffer(Device.physicalDevice, Device.device, commandPool, Queues.graphicsQueue, indexBuffer, indexBufferMemory, indices);
    createCommandBuffer(commandPool, Device.device, SwapChain.swapChainImages.size(), commandBuffers);   
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        recordCommandBuffer(
            commandBuffers[i],
            static_cast<uint32_t>(i),
            renderPass,
            swapChainFramebuffers,
            SwapChain.swapChainExtent,
            graphicsPipeline,
            vertexBuffer,
            indexBuffer,
            indices,
            pipelineLayout,
            descriptorSets
        );

    }
    createSyncObjects(Device.device, imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences, MAX_FRAMES_IN_FLIGHT);

    bool running = true;

    SDL_Event e;

    while (running) {

        while (SDL_PollEvent(&e)) {

            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                framebufferResized = true;
            }
        }

        drawFrame(
            Device.device,
            SwapChain.swapChain,
            renderPass,
            swapChainFramebuffers,
            SwapChain.swapChainExtent,
            graphicsPipeline,
            Queues.graphicsQueue,
            Queues.presentQueue,
            inFlightFences,
            renderFinishedSemaphores,
            imageAvailableSemaphores,
            commandBuffers,
            MAX_FRAMES_IN_FLIGHT,
            Surface.surface,
            Window.window,
            Device.physicalDevice,
            commandPool,
            SwapChain.swapChainImageFormat,
            SwapChain.swapChainImages,
            swapChainImageViews,
            pipelineLayout,
            vertexBuffer,
            vertices,
            indexBuffer,
            descriptorSetLayout,
            uniformBuffersMapped,
            descriptorSets,
            depthImage,
            depthImageMemory,
            depthImageView,
            imagesInFlight
        );
    }

    
    // TODO: Fix cleanup
    vkDeviceWaitIdle(Device.device);   
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(Device.device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(Device.device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(Device.device, inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(Device.device, commandPool, nullptr);
    vkDestroyPipelineLayout(Device.device, pipelineLayout, nullptr);
    cleanupSwapChain(Device.device, renderPass, swapChainFramebuffers, commandPool, commandBuffers, swapChainImageViews, SwapChain.swapChain);
    vkDestroyImageView(Device.device, depthImageView, nullptr);
    vkDestroyImage(Device.device, depthImage, nullptr);
    vkFreeMemory(Device.device, depthImageMemory, nullptr);
    vkDestroySampler(Device.device, textureSampler, nullptr);
    vkDestroyImageView(Device.device, textureImageView, nullptr);
    vkDestroyImage(Device.device, textureImage, nullptr);
    vkFreeMemory(Device.device, textureImageMemory, nullptr);         
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(Device.device, uniformBuffers[i], nullptr);
        vkFreeMemory(Device.device, uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(Device.device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(Device.device, descriptorSetLayout, nullptr);
    vkDestroyBuffer(Device.device, vertexBuffer, nullptr);
    vkFreeMemory(Device.device, vertexBufferMemory, nullptr);
    vkDestroyBuffer(Device.device, indexBuffer, nullptr);
    vkFreeMemory(Device.device, indexBufferMemory, nullptr);
    vkDestroyDevice(Device.device, nullptr);
    vkDestroySurfaceKHR(Instance.instance, Surface.surface, nullptr);
    cleanupInstance();
    SDL_DestroyWindow(Window.window);
    // SDL_Terminate
    return 0;
}

