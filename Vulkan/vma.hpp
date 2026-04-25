#pragma once

#include "vulkan.h"
#include "VulkanUtils.h"

namespace vma {
	struct Buffer {
		vk::Buffer buffer;
		VmaAllocation allocation;
	};

	struct Image {
		vk::Image image;
		vk::ImageView imageView;

		vk::ImageCreateInfo info;

		VmaAllocation allocation;
	};

	struct Allocator {
		Allocator() = default;
		Allocator(const Allocator& allocator) : allocator(allocator.allocator) {}
		Allocator(const VmaAllocator& allocator) : allocator(allocator) {}

		Allocator& operator=(const Allocator& other) {
			allocator = other.allocator;
			return *this;
		}

		Allocator& operator=(const VmaAllocator& other) {
			allocator = other;
			return *this;
		}

		operator VmaAllocator() const noexcept {
			return allocator;
		}

		operator bool() const noexcept {
			return allocator != VK_NULL_HANDLE;
		}

		Result<vma::Buffer> createBuffer(const vk::BufferCreateInfo& bufferInfo, const VmaAllocationCreateInfo& allocationInfo) const {
			vma::Buffer buffer = {};

			auto result = vmaCreateBuffer(allocator,
				reinterpret_cast<const VkBufferCreateInfo*>( &bufferInfo ), &allocationInfo,
				reinterpret_cast<VkBuffer*>( &buffer.buffer ), &buffer.allocation, 
				nullptr);

			if (result != VK_SUCCESS) {
				return result;
			}

			return buffer;
		}

		Result<vma::Image> createImage(const vk::ImageCreateInfo& imageInfo, const VmaAllocationCreateInfo& allocationInfo) const {
			vma::Image image = {};

			auto result = vmaCreateImage(allocator,
				reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &allocationInfo,
				reinterpret_cast<VkImage*>(&image.image), &image.allocation,
				nullptr);

			if (result != VK_SUCCESS) {
				return result;
			}

			image.info = imageInfo;

			return image;
		}

		void destroyBuffer(vk::Buffer buffer, VmaAllocation allocation) const {
			vmaDestroyBuffer(allocator, buffer, allocation);
		}

		void destroyImage(vk::Image image, VmaAllocation allocation) const {
			vmaDestroyImage(allocator, image, allocation);
		}

		void destroyBuffer(vma::Buffer buffer) const {
			vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
		}

		void destroyImage(vma::Image image) const {
			vmaDestroyImage(allocator, image.image, image.allocation);
		}

		void destroy() const {
			vmaDestroyAllocator(allocator);
		}

		vk::Result mapMemory(VmaAllocation allocation, void** ppData) const {
			return static_cast<vk::Result>( vmaMapMemory(allocator, allocation, ppData) );
		}

		void unmapMemory(VmaAllocation allocation) const {
			vmaUnmapMemory(allocator, allocation);
		}

		VmaAllocator allocator;
	};

	Result<vma::Allocator> createAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device);
}