#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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

// doccument everything

bool framebufferResized = false;
uint32_t currentFrame = 0;
uint32_t imageIndex;

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 0.3f}, {0.0f, 1.0f}},
    {{ 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 0.3f}, {1.0f, 1.0f}},
    {{ 0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 0.3f}, {1.0f, 0.0f}},
    {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 0.3f}, {0.0f, 0.0f}}
};


const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    framebufferResized = true;
}

void drawFrame(
    VkDevice& device, 
    VkFence& inFlightFence, 
    VkSwapchainKHR& swapChain, 
    VkSemaphore& imageAvailableSemaphore, 
    VkCommandBuffer& commandBuffer, 
    VkRenderPass& renderPass,
    std::vector<VkFramebuffer>& swapChainFramebuffers, 
    VkExtent2D& swapChainExtent, 
    VkPipeline& graphicsPipeline, 
    VkQueue& graphicsQueue, 
    VkQueue& presentQueue, 
    VkSemaphore& renderFinishedSemaphore, 
    std::vector<VkFence>& inFlightFences, 
    std::vector<VkSemaphore>& renderFinishedSemaphores, 
    std::vector<VkSemaphore>& imageAvailableSemaphores, 
    std::vector<VkCommandBuffer>& commandBuffers, 
    int MAX_FRAMES_IN_FLIGHT, VkSurfaceKHR surface, 
    GLFWwindow* window, 
    VkPhysicalDevice& physicalDevice, 
    VkCommandPool& commandPool, 
    VkFormat& swapChainImageFormat, 
    std::vector<VkImage>& swapChainImages, 
    std::vector<VkImageView>& swapChainImageViews, 
    VkPipelineLayout& pipelineLayout,
    VkBuffer vertexBuffer,
    std::vector<Vertex> vertices, 
    VkBuffer indexBuffer,
    VkDescriptorSetLayout descriptorSetLayout,
    std::vector<void*> uniformBuffersMapped,
    std::vector<VkDescriptorSet> descriptorSets,
    VkImage depthImage,
    VkDeviceMemory depthImageMemory,
    VkImageView depthImageView
    ) {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
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
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    updateUniformBuffer(currentFrame, uniformBuffersMapped, swapChainExtent);

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

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
        return;
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

int main() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to init GLFW!");
    } else {
        std::cout << "Initialized GLFW correctly!" << std::endl;
    }   
    GLFWwindow* window;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkDevice device = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;  
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
    
    createWindow(800, 600, "Vulkan Window", window);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    createInstance();
    setupDebugMessenger(Instance.instance);
    createSurface(Instance.instance, surface, window);
    pickPhysicalDevice(Instance.instance, physicalDevice, surface);
    createLogicalDevice(physicalDevice, surface, device, graphicsQueue, presentQueue);
    createSwapChain(physicalDevice, surface, window, swapChain, device, swapChainImages, swapChainImageFormat, swapChainExtent);
    createImageViews(swapChainImageViews, swapChainImages, swapChainImageFormat, device);
    createRenderPass(swapChainImageFormat, renderPass, device, physicalDevice);
    createDescriptorSetLayout(device, descriptorSetLayout);
    createGraphicsPipeline(device, swapChainExtent, pipelineLayout, renderPass, graphicsPipeline, descriptorSetLayout);
    createCommandPool(physicalDevice, surface, commandPool, device);
    createDepthResources(swapChainExtent, depthImageView, depthImage, depthImageMemory, device, physicalDevice, commandPool, graphicsQueue);
    createFramebuffers(swapChainFramebuffers, swapChainImageViews, renderPass, swapChainExtent, device, depthImageView);
    createTextureImage(physicalDevice, device, textureImage, textureImageMemory, graphicsQueue, commandPool);
    createTextureImageView(device, textureImage, textureImageView);
    createTextureSampler(physicalDevice, textureSampler, device);
    createVertexBuffer(
        physicalDevice,
        device,
        commandPool,
        graphicsQueue,
        vertexBuffer,
        vertexBufferMemory,
        vertices
    );
    createUniformBuffers(MAX_FRAMES_IN_FLIGHT, device, uniformBuffers, uniformBuffersMemory, uniformBuffersMapped, physicalDevice);
    createDescriptorPool(MAX_FRAMES_IN_FLIGHT, descriptorPool, device);
    createDescriptorSets(MAX_FRAMES_IN_FLIGHT, descriptorPool, descriptorSetLayout, descriptorSets, device, uniformBuffers, textureSampler, textureImageView);
    createIndexBuffer(physicalDevice, device, commandPool, graphicsQueue, indexBuffer, indexBufferMemory, indices);
    createCommandBuffer(commandPool, device, swapChainImages.size(), commandBuffers);   
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        recordCommandBuffer(
            commandBuffers[i],
            static_cast<uint32_t>(i),
            renderPass,
            swapChainFramebuffers,
            swapChainExtent,
            graphicsPipeline,
            vertexBuffer,
            indexBuffer,
            indices,
            pipelineLayout,
            descriptorSets
        );

    }
    createSyncObjects(device, imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences, MAX_FRAMES_IN_FLIGHT);    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        drawFrame(device, inFlightFence, swapChain, imageAvailableSemaphore, commandBuffer, renderPass, swapChainFramebuffers, swapChainExtent, graphicsPipeline, graphicsQueue, presentQueue, renderFinishedSemaphore, inFlightFences, renderFinishedSemaphores, imageAvailableSemaphores, commandBuffers, MAX_FRAMES_IN_FLIGHT, surface, window, physicalDevice, commandPool, swapChainImageFormat, swapChainImages, swapChainImageViews, pipelineLayout, vertexBuffer, vertices, indexBuffer, descriptorSetLayout, uniformBuffersMapped, descriptorSets, depthImage, depthImageMemory, depthImageView);
    }
    
    // TODO: Fix cleanup
    vkDeviceWaitIdle(device);   
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(device, commandPool, nullptr);
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    cleanupSwapChain(device, renderPass, swapChainFramebuffers, commandPool, commandBuffers, swapChainImageViews, swapChain);
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);
    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);         
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(Instance.instance, surface, nullptr);
    cleanupInstance(Instance.instance);
    glfwDestroyWindow(window);
    glfwTerminate();
}

