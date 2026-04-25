#pragma once

#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "vma.hpp"

struct VkCustomSwapchain {
    void destroy(VkCustomDevice& device);

    void createFramebuffer(VkCustomDevice& device, vk::RenderPass renderPass);

    vk::SwapchainKHR swapchain{};
    vk::Format swapchainImageFormat{};
    vk::Extent2D extentImage{};
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::ImageView> swapchainImageViews;
    std::vector<vk::Framebuffer> framebuffers;
    
    vk::Format depthFormat{};
    vma::Image depthImage;

    vma::Allocator allocator;
};

class SwapchainBuilder {
public:
    SwapchainBuilder(VkCustomDevice& device, VkExtent2D extend);

    VkCustomSwapchain createSwapchain(vma::Allocator allocator, vk::SurfaceKHR surface, uint32_t presentQueueIndex, uint32_t graphicQueueIndex);
private:
    VkCustomDevice& device;

    vk::SwapchainCreateInfoKHR createInfo{};
    vk::SurfaceFormatKHR preferedSurfaceFormat;
    vk::PresentModeKHR preferedPresentMode;
    vk::Extent2D preferedExtent;
};