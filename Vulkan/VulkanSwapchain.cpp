#pragma once

#include "VulkanSwapchain.h"


void VkCustomSwapchain::destroy(VkCustomDevice& device) {
    vk::Device vkDevice = device.get();

    for (auto& imageView : swapchainImageViews)
        vkDevice.destroyImageView(imageView);
    for (auto& framebuffer : framebuffers)
        vkDevice.destroyFramebuffer(framebuffer);

    vkDevice.destroyImageView(depthImage.imageView);
    allocator.destroyImage(depthImage);

    vkDevice.destroySwapchainKHR(swapchain);
}

void VkCustomSwapchain::createFramebuffer(VkCustomDevice& device, vk::RenderPass renderPass) {
    framebuffers.resize(swapchainImages.size());
    for (int i = 0; i < swapchainImages.size(); i++) {
        vk::ImageView attachments[] = { swapchainImageViews[i], depthImage.imageView };
        vk::FramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.width = extentImage.width;
        framebufferCreateInfo.height = extentImage.height;
        framebufferCreateInfo.layers = 1;
        framebufferCreateInfo.setAttachments(attachments);
        framebufferCreateInfo.renderPass = renderPass;

        framebuffers[i] = device.get().createFramebuffer(framebufferCreateInfo);
    }
}

SwapchainBuilder::SwapchainBuilder(VkCustomDevice& device, VkExtent2D extend) : device(device), preferedExtent(extend) {
    preferedSurfaceFormat.format = vk::Format::eB8G8R8A8Srgb;
    preferedSurfaceFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    preferedPresentMode = vk::PresentModeKHR::eMailbox;
}

VkCustomSwapchain SwapchainBuilder::createSwapchain(vma::Allocator allocator, vk::SurfaceKHR surface, uint32_t presentQueueIndex, uint32_t graphicQueueIndex) {
    vk::PhysicalDevice physicalDevice = device.getPhysicalDevice();
    vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    std::vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(surface);
    std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

    vk::SurfaceFormatKHR surfaceFormat = vkUtils::chooseSwapSurfaceFormat(formats, preferedSurfaceFormat);
    vk::PresentModeKHR presentMode = vkUtils::chooseSwapPresentMode(presentModes, preferedPresentMode);
    vk::Extent2D extent = vkUtils::chooseSwapExtent(capabilities, preferedExtent.width, preferedExtent.height);

    size_t imageCount = (size_t)capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    uint32_t queueFamilyIndices[] = { graphicQueueIndex, presentQueueIndex };

    if (graphicQueueIndex != presentQueueIndex) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.setQueueFamilyIndices(queueFamilyIndices);
    }
    else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkCustomSwapchain swapchainInfo;

    swapchainInfo.swapchain = device.get().createSwapchainKHR(createInfo);
    swapchainInfo.swapchainImages = device.get().getSwapchainImagesKHR(swapchainInfo.swapchain);

    imageCount = swapchainInfo.swapchainImages.size();
    swapchainInfo.swapchainImageViews.resize(imageCount);

    swapchainInfo.swapchainImageFormat = surfaceFormat.format;
    swapchainInfo.extentImage = extent;

    for (size_t i = 0; i < imageCount; i++) {
        vk::ImageViewCreateInfo imageViewCreateInfo = vkUtils::imageviewCreateInfo(
            swapchainInfo.swapchainImageFormat,
            swapchainInfo.swapchainImages[i],
            vk::ImageAspectFlagBits::eColor
        );

        imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
        imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
        imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
        imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;

        swapchainInfo.swapchainImageViews[i] = device.get().createImageView(imageViewCreateInfo);
    }

    VkExtent3D depthImageExtent = { extent.width, extent.height, 1 };

    swapchainInfo.depthFormat = vk::Format::eD32Sfloat;

    //the depth image will be an image with the format we selected and Depth Attachment usage flag
    VkImageCreateInfo dimg_info = vkUtils::imageCreateInfo(
        swapchainInfo.depthFormat,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        depthImageExtent
    );

    //for the depth image, we want to allocate it from GPU local memory
    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    auto imageResult = allocator.createImage(dimg_info, dimg_allocinfo);
    swapchainInfo.depthImage = imageResult.getValue();

    //build an image-view for the depth image to use for rendering
    VkImageViewCreateInfo dview_info = vkUtils::imageviewCreateInfo(
        swapchainInfo.depthFormat,
        swapchainInfo.depthImage.image,
        vk::ImageAspectFlagBits::eDepth
    );

    swapchainInfo.depthImage.imageView = device.get().createImageView(dview_info);
    swapchainInfo.allocator = allocator;

    return swapchainInfo;
}