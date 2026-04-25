#pragma once

#include "VulkanSwapchain.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanObject.h"

#include <glm/gtx/transform.hpp>


class VulkanContext {
public:
	VulkanContext(GLFWwindow* window);

	void init(GLFWwindow* window);

	~VulkanContext();

	void draw();

	void setCamera(glm::vec3 position, glm::vec3 direction);

public:
	GLFWwindow* window;

	VkCustomInstance instance;
	VkCustomDevice device;

	vk::SurfaceKHR surface;
	vk::RenderPass renderPass;
	VkCustomSwapchain swapchain;

	uint32_t allQueueIndex;
	vk::Queue allQueue;
	vk::Queue presentQueue;

	vk::CommandPool commandPool;
	std::vector<vk::CommandBuffer> mainCommandBuffer;

	vk::Semaphore presentSemaphore, renderSemaphore;
	vk::Fence renderFence;

	vk::Pipeline trianglePipeline;
	vk::PipelineLayout pipelineLayout;

	vma::Allocator allocator;

	Mesh triangleMesh;
	Camera camera;
};
