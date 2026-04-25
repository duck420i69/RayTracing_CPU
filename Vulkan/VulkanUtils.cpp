#include "VulkanUtils.h"
#include "VulkanImage.h"

// (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)

namespace vkUtils {
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats, vk::SurfaceFormatKHR preferFormat) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == preferFormat.format && preferFormat.colorSpace == preferFormat.colorSpace) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes, vk::PresentModeKHR preferPresentMode) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == preferPresentMode) {
                return availablePresentMode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, int width, int height) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }


    Result<vk::RenderPass> initDefaultRenderpass(vk::Device device, vk::Format swapchainImageFormat, vk::Format depthFormat)
    {
        try {
            // the renderpass will use this color attachment.
            vk::AttachmentDescription color_attachment = {};

            //the attachment will have the format needed by the swapchain
            color_attachment.format = swapchainImageFormat;

            //1 sample, we won't be doing MSAA
            color_attachment.samples = vk::SampleCountFlagBits::e1;

            // we Clear when this attachment is loaded
            color_attachment.loadOp = vk::AttachmentLoadOp::eClear;

            // we keep the attachment stored when the renderpass ends
            color_attachment.storeOp = vk::AttachmentStoreOp::eStore;

            //we don't care about stencil
            color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

            //we don't know or care about the starting layout of the attachment
            color_attachment.initialLayout = vk::ImageLayout::eUndefined;

            //after the renderpass ends, the image has to be on a layout ready for display
            color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

            vk::AttachmentDescription depth_attachment = vk::AttachmentDescription(
                {},
                depthFormat, vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal
            );

            //attachment number will index into the pAttachments array in the parent renderpass itself
            vk::AttachmentReference color_attachment_ref = {};
            color_attachment_ref.attachment = 0;
            color_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

            vk::AttachmentReference depth_attachment_ref = {};
            depth_attachment_ref.attachment = 1;
            depth_attachment_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

            //we are going to create 1 subpass, which is the minimum you can do
            vk::SubpassDescription subpass = {};
            subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_attachment_ref;
            subpass.pDepthStencilAttachment = &depth_attachment_ref;

            vk::SubpassDependency dependency = vk::SubpassDependency(
                VK_SUBPASS_EXTERNAL, 0,
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite,
                {}
            );

            vk::SubpassDependency depth_dependency = vk::SubpassDependency(
                VK_SUBPASS_EXTERNAL, 0,
                vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, 
                vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                vk::AccessFlagBits::eNone, vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                {}
            );

            //array of 2 attachments, one for the color, and other for depth
            vk::AttachmentDescription attachments[] = { color_attachment,depth_attachment };
            vk::SubpassDependency dependencies[] = { dependency, depth_dependency };
            vk::RenderPassCreateInfo render_pass_info = {};

            render_pass_info.setAttachments(attachments);
            render_pass_info.setSubpasses(subpass);
            render_pass_info.setDependencies(dependencies);

            return device.createRenderPass(render_pass_info);
        }
        catch (const vk::SystemError& err) {
            return err;
        }
    }

    vk::ImageCreateInfo imageCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent3D extent)
    {
        vk::ImageCreateInfo info = {};

        info.imageType = vk::ImageType::e2D;

        info.format = format;
        info.extent = extent;

        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = vk::SampleCountFlagBits::e1;
        info.tiling = vk::ImageTiling::eOptimal;
        info.usage = usageFlags;

        return info;
    }

    vk::ImageViewCreateInfo imageviewCreateInfo(vk::Format format, vk::Image image, vk::ImageAspectFlags aspectFlags) {
        //build a image-view for the depth image to use for rendering
        vk::ImageViewCreateInfo info = {};

        info.viewType = vk::ImageViewType::e2D;
        info.image = image;
        info.format = format;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        info.subresourceRange.aspectMask = aspectFlags;

        return info;
    }
}