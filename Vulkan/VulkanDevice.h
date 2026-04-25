#pragma once

#include "VulkanInstance.h"
#include "VulkanUtils.h"


enum class VkQueueType {
    ALL_PORPOSE,
    GRAPHIC,
    COMPUTE,
    TRANSFER,
    TRASFER_ONLY,
    CONPUTE_ONLY
};


std::optional<uint32_t> getQueueIndex(vk::PhysicalDevice physicalDevice, VkQueueType type, int startIndex = 0);
std::optional<uint32_t> getPresentQueue(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);


class VkCustomDevice {
public:
    VkCustomDevice() {}
    VkCustomDevice(VkPhysicalDevice physicalDevice, VkDevice device, std::vector<std::pair<VkQueueType, uint32_t>> queueIndexes, std::optional<uint32_t> presentIndex)
        : physicalDevice(physicalDevice), device(device), queueIndexes(queueIndexes), presentIndex(presentIndex) {}
    
    VkCustomDevice(const VkCustomDevice& device) = delete;
    VkCustomDevice& operator=(const VkCustomDevice& device) = delete;
    
    VkCustomDevice(VkCustomDevice&& device) noexcept;
    VkCustomDevice& operator=(VkCustomDevice&& device) noexcept;

    ~VkCustomDevice() {
        vkDestroyDevice(device, nullptr);
    }

    vk::Device get() { return device; }
    
    vk::PhysicalDevice getPhysicalDevice() { return physicalDevice; }
    
    std::vector<std::pair<VkQueueType, uint32_t>> getQueueIndexList() { return queueIndexes; }
    
    uint32_t getQueueIndex(VkQueueType type) {
        for (auto& queue : queueIndexes) {
            if (queue.first == type)
                return queue.second;
        }
        throw std::runtime_error("Queue is not available because it have not been requested during create device.");
        return 0;
    }
    
    uint32_t getPresentIndex() {
        return presentIndex.value();
    }
    
    vk::Queue getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex = 0) {
        return device.getQueue(queueFamilyIndex, queueIndex);
    }

    std::optional<vk::ShaderModule> loadShader(const char* filePath);
private:
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    std::vector<std::pair<VkQueueType, uint32_t>> queueIndexes;
    std::optional<uint32_t> presentIndex;
};


class DeviceBuilder {
public:
    DeviceBuilder(VkCustomInstance& instance);

    DeviceBuilder& addQueue(VkQueueType type, uint32_t queueCount = 1, float queuePriority = 1.0f);

    DeviceBuilder& addExtention(const char* extension);

    DeviceBuilder& addExtentions(std::vector<const char*> extensions);

    DeviceBuilder& choosePhysicalDevice();

    DeviceBuilder& requestSwapchainSupport(VkSurfaceKHR surface);

    Result<VkCustomDevice> build();

private:
    VkCustomInstance& instance;
    vk::PhysicalDevice physicalDevice;
    
    std::vector<const char*> deviceExtensions;
    std::vector<VkQueueType> queueTypeRequired;
    std::vector<vk::PhysicalDevice> suitableDevices;
    std::vector<vk::PhysicalDevice> listDevices;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    VkSurfaceKHR surface{};
    
    bool presentSupport = false;

    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

    bool isDeviceSuitable(vk::PhysicalDevice device);

};