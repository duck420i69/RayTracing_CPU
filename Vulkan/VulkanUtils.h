#pragma once

#include "vulkan.h"


template<class T>
struct Result {
    T value;
    std::optional<vk::SystemError> err;

    Result() {}
    Result(T&& value) : value(std::move(value)) {}
    Result(vk::SystemError err) : err(err) {}
    Result(VkResult result) {
        try {
            vk::resultCheck(static_cast<vk::Result>( result ), "");
        }
        catch (vk::SystemError err) {
            this->err = err;
        }
    }

    T&& getValue() {
        if (err.has_value()) {   
            std::cerr << "\ngetValue error:\ncode " << err.value().what() << "\n\n";
            throw err;
        }
            
        return std::move(value);
    }
};


namespace vkUtils {
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats, vk::SurfaceFormatKHR preferFormat);

    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes, vk::PresentModeKHR preferPresentMode);

    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, int width, int height);

    Result<vk::RenderPass> initDefaultRenderpass(vk::Device device, vk::Format swapchainImageFormat, vk::Format depthFormat);

    vk::ImageCreateInfo imageCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent3D extent);

    vk::ImageViewCreateInfo imageviewCreateInfo(vk::Format format, vk::Image image, vk::ImageAspectFlags aspectFlags);
}