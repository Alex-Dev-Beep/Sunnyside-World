#include "device.hpp"
#include "validation_layers.hpp"
#include "swapchain.hpp"
#include "vertex.hpp"

#include <stdexcept>
#include <vector>
#include <iostream>
#include <string>
#include <set>

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, queueFamilies.data()
    );

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {

        // Graphics queue
        if (queueFamily.queueCount > 0 &&
            queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device,
            i,
            surface,
            &presentSupport
        );

        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice& device, VkQueue& graphicsQueue, VkQueue& presentQueue) {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;

    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;

    createInfo.pEnabledFeatures = &deviceFeatures;createInfo.enabledExtensionCount = 0;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    } else {
        std::cout << "Succesfully created Logical Device!" << std::endl;
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport =
            querySwapChainSupport(device, surface);

        swapChainAdequate =
            !swapChainSupport.formats.empty() &&
            !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

void pickPhysicalDevice(VkInstance instance, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        std::cout << "Checking GPU: "
                << props.deviceName << std::endl;

        if (isDeviceSuitable(device, surface)) {
            physicalDevice = device;
            std::cout << "Selected GPU!" << std::endl;
            break;
        }
    }


    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    std::cout << "Using GPU: " << props.deviceName << std::endl;

}

void createCommandPool(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool& commandPool, VkDevice device) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }

    std::cout << "Succesfully created command pool!" << std::endl;
}

void createCommandBuffer(VkCommandPool commandPool, VkDevice device, int MAX_FRAMES_IN_FLIGHT, std::vector<VkCommandBuffer>& commandBuffers) {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount =
        static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(
            device,
            &allocInfo,
            commandBuffers.data()
        ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    std::cout << "Succesfully allocated command buffers!" << std::endl;
}

void recordCommandBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex,
    VkRenderPass renderPass,
    const std::vector<VkFramebuffer>& swapChainFramebuffers,
    VkExtent2D swapChainExtent,
    VkPipeline graphicsPipeline,
    VkBuffer vertexBuffer,
    VkBuffer indexBuffer,
    const std::vector<uint16_t>& indices,
    VkPipelineLayout pipelineLayout,
    const std::vector<VkDescriptorSet>& descriptorSets
) {

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin command buffer");
    }

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChainExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline
    );

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(
        commandBuffer,
        indexBuffer,
        0,
        VK_INDEX_TYPE_UINT16
    );

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width  = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0,
        1,
        &descriptorSets[imageIndex],
        0,
        nullptr
    );

    vkCmdDrawIndexed(
        commandBuffer,
        static_cast<uint32_t>(indices.size()),
        1,
        0,
        0,
        0
    );

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer");
    }

    std::cout << "Successfully recorded command buffer!" << std::endl;
}
