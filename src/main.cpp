#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_keyboard.h>
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
#include "flight.hpp"
#include "render_pass.hpp"

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
    std::vector<VkFence>& imagesInFlight,
    UniformBufferObject ubo
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

    updateUniformBuffer(imageIndex, uniformBuffersMapped, swapChainExtent, ubo);


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
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK)) {
        throw std::runtime_error("Failed to init SDL!");
    } else {
        std::cout << "Initialized SDL correctly!" << std::endl;
    }
    VkPipelineLayout pipelineLayout;
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
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkDescriptorPool descriptorPool;    
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    UniformBufferObject ubo{};

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    SDL_Gamepad* pad = nullptr;
    glm::vec2 playerPos = {0.0f, 0.0f};
    float speed = 1.5f; // unidades por segundo

    createWindow();
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool(Device.physicalDevice, Surface.surface, commandPool, Device.device);
    createDepthResources(SwapChain.swapChainExtent, depthImageView, depthImage, depthImageMemory, Device.device, Device.physicalDevice, commandPool, Queues.graphicsQueue);
    createFramebuffers(swapChainFramebuffers, SwapChain.swapChainImageViews, RenderPass.renderPass, SwapChain.swapChainExtent, Device.device, depthImageView);
    createTextureImage(Device.physicalDevice, Device.device, textureImage, textureImageMemory, Queues.graphicsQueue, commandPool);
    createTextureImageView(Device.device, textureImage, textureImageView);
    createTextureSampler(Device.physicalDevice, textureSampler, Device.device);
    createVertexBuffer(Device.physicalDevice, Device.device, commandPool, Queues.graphicsQueue, vertexBuffer, vertexBufferMemory, vertices);
    createUniformBuffers(FlightFrames.MAX_FRAMES_IN_FLIGHT, Device.device, uniformBuffers, uniformBuffersMemory, uniformBuffersMapped, Device.physicalDevice);
    createDescriptorPool(FlightFrames.MAX_FRAMES_IN_FLIGHT, descriptorPool, Device.device);
    createDescriptorSets(FlightFrames.MAX_FRAMES_IN_FLIGHT, descriptorPool, DescriptorSetLayouts.descriptorSetLayout, descriptorSets, Device.device, uniformBuffers, textureSampler, textureImageView);
    createIndexBuffer(Device.physicalDevice, Device.device, commandPool, Queues.graphicsQueue, indexBuffer, indexBufferMemory, indices);
    createCommandBuffer(commandPool, Device.device, SwapChain.swapChainImages.size(), commandBuffers);   
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        recordCommandBuffer(
            commandBuffers[i],
            static_cast<uint32_t>(i),
            RenderPass.renderPass,
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
    createSyncObjects(Device.device, imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences, FlightFrames.MAX_FRAMES_IN_FLIGHT);

    bool running = true;
    SDL_Event e;

    bool playerIsMoving = false;
    Uint64 lastCounter = SDL_GetPerformanceCounter();
    while (running) {
        Uint64 currentCounter = SDL_GetPerformanceCounter();
        float deltaTime =
        (currentCounter - lastCounter) /
        (float)SDL_GetPerformanceFrequency();
        lastCounter = currentCounter;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT)
                running = false;
        }

        const bool* keys = SDL_GetKeyboardState(NULL);

        if (keys[SDL_SCANCODE_W]) playerPos.y += speed * deltaTime;
        if (keys[SDL_SCANCODE_S]) playerPos.y -= speed * deltaTime;
        if (keys[SDL_SCANCODE_A]) playerPos.x -= speed * deltaTime;
        if (keys[SDL_SCANCODE_D]) playerPos.x += speed * deltaTime;

        if (pad) {
            float lx = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX) / 32767.0f;
            float ly = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY) / 32767.0f;

            const float DEADZONE = 0.15f;

            if (fabs(lx) > DEADZONE)
                playerPos.x += lx * speed * deltaTime;
            if (fabs(ly) > DEADZONE)
                playerPos.y -= ly * speed * deltaTime;
        }

        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(playerPos, 0.0f));
        
        drawFrame(
            Device.device,
            SwapChain.swapChain,
            RenderPass.renderPass,
            swapChainFramebuffers,
            SwapChain.swapChainExtent,
            graphicsPipeline,
            Queues.graphicsQueue,
            Queues.presentQueue,
            inFlightFences,
            renderFinishedSemaphores,
            imageAvailableSemaphores,
            commandBuffers,
            FlightFrames.MAX_FRAMES_IN_FLIGHT,
            Surface.surface,
            Window.window,
            Device.physicalDevice,
            commandPool,
            SwapChain.swapChainImageFormat,
            SwapChain.swapChainImages,
            SwapChain.swapChainImageViews,
            pipelineLayout,
            vertexBuffer,
            vertices,
            indexBuffer,
            DescriptorSetLayouts.descriptorSetLayout,
            uniformBuffersMapped,
            descriptorSets,
            depthImage,
            depthImageMemory,
            depthImageView,
            FlightFrames.imagesInFlight,
            ubo
        );
    }

    
    // TODO: Fix cleanup
    vkDeviceWaitIdle(Device.device);   
    for (size_t i = 0; i < FlightFrames.MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(Device.device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(Device.device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(Device.device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(Device.device, commandPool, nullptr);
    vkDestroyPipelineLayout(Device.device, pipelineLayout, nullptr);
    cleanupSwapChain(Device.device, RenderPass.renderPass, swapChainFramebuffers, commandPool, commandBuffers, SwapChain.swapChainImageViews, SwapChain.swapChain);
    vkDestroyImageView(Device.device, depthImageView, nullptr);
    vkDestroyImage(Device.device, depthImage, nullptr);
    vkFreeMemory(Device.device, depthImageMemory, nullptr);
    vkDestroySampler(Device.device, textureSampler, nullptr);
    vkDestroyImageView(Device.device, textureImageView, nullptr);
    vkDestroyImage(Device.device, textureImage, nullptr);
    vkFreeMemory(Device.device, textureImageMemory, nullptr);         
    for (size_t i = 0; i < FlightFrames.MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(Device.device, uniformBuffers[i], nullptr);
        vkFreeMemory(Device.device, uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(Device.device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(Device.device, DescriptorSetLayouts.descriptorSetLayout, nullptr);
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

