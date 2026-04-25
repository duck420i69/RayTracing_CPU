#pragma once

#include "VulkanUtils.h"


class VkCustomInstance {
public:
    VkCustomInstance() : VkCustomInstance(VK_NULL_HANDLE) {};
    VkCustomInstance(vk::Instance instance);

    VkCustomInstance(const VkCustomInstance& instance) = delete;
    VkCustomInstance(VkCustomInstance&& instance) noexcept;
    
    VkCustomInstance& operator=(const VkCustomInstance& other) = delete;
    VkCustomInstance& operator=(VkCustomInstance&& other) noexcept;
    
    ~VkCustomInstance();
    
    vk::Instance get();
    
    Result<vk::SurfaceKHR> createSurface(GLFWwindow* window);
private:
    vk::Instance instance;
    vk::SurfaceKHR surface;
    VkDebugUtilsMessengerEXT debugMessenger;
};


class InstanceBuilder {
public:
    InstanceBuilder();
    InstanceBuilder& setAppName(char* name);
    InstanceBuilder& setEngineName(char* name);
    InstanceBuilder& setAppVersion(int major, int minor, int patch);
    InstanceBuilder& setAPIVersion(int major, int minor, int patch);
    InstanceBuilder& setEngineVersion(int major, int minor, int patch);
    InstanceBuilder& addExtention(const char* extension);
    InstanceBuilder& addExtentions(std::vector<const char*> extensions);

    Result<VkCustomInstance> build();

private:
    std::vector<const char*> validation_layers;
    std::vector<const char*> required_extensions;
    vk::ApplicationInfo appInfo{};
    vk::InstanceCreateInfo insInfo{};
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
};