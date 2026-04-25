#include "vma.hpp"

Result<vma::Allocator> vma::createAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device) {
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;

	VmaAllocator allocator;
	auto result = vmaCreateAllocator(&allocatorInfo, &allocator);

	if (result != VK_SUCCESS) {
		return result;
	}

	return vma::Allocator(allocator);
}