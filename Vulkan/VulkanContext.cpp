#pragma once

#include "VulkanContext.h"

#include "loader.h" // remove this


VulkanContext::VulkanContext(GLFWwindow* window) {
	init(window);
}

void VulkanContext::init(GLFWwindow* window) {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}


		auto instanceResult = InstanceBuilder().addExtentions(extensions).build();
		instance = std::move(instanceResult.value);


		auto surfaceResult = instance.createSurface(window);
		surface = surfaceResult.getValue();


		int window_width, window_height;
		glfwGetWindowSize(window, &window_width, &window_height);


		auto deviceResult = DeviceBuilder(instance)
			.addQueue(VkQueueType::ALL_PORPOSE)
			.requestSwapchainSupport(surface)
			.build();

		device = std::move(deviceResult.getValue());
		allQueueIndex = device.getQueueIndex(VkQueueType::ALL_PORPOSE);
		allQueue = device.getQueue(allQueueIndex);


		auto allocatorResult = vma::createAllocator(instance.get(), device.getPhysicalDevice(), device.get());
		allocator = allocatorResult.getValue();


		commandPool = device.get().createCommandPool(
			vk::CommandPoolCreateInfo(
				vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				allQueueIndex
			));

		mainCommandBuffer = device.get().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(
				commandPool,
				vk::CommandBufferLevel::ePrimary,
				1
			));


		swapchain = SwapchainBuilder(
			device, VkExtent2D{ (uint32_t)window_width, (uint32_t)window_height })
			.createSwapchain(allocator, surface, allQueueIndex, allQueueIndex);


		Result<vk::RenderPass> renderPassResult = vkUtils::initDefaultRenderpass(device.get(), swapchain.swapchainImageFormat, swapchain.depthFormat);
		renderPass = renderPassResult.getValue();

		swapchain.createFramebuffer(device, renderPass);


		//we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
		vk::FenceCreateInfo fenceCreateInfo = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);

		renderFence = device.get().createFence(fenceCreateInfo);

		//for the semaphores we don't need any flags
		vk::SemaphoreCreateInfo semaphoreCreateInfo = {};
		presentSemaphore = device.get().createSemaphore(semaphoreCreateInfo);
		renderSemaphore = device.get().createSemaphore(semaphoreCreateInfo);


		VkShaderModule triangleVertexShader, triangleFragShader;
		auto tempShader = device.loadShader("shaders/vert.spv");
		if (!tempShader.has_value())
			throw std::runtime_error("fail to load shader");
		triangleVertexShader = tempShader.value();

		tempShader = device.loadShader("shaders/frag.spv");
		if (!tempShader.has_value())
			throw std::runtime_error("fail to load shader");
		triangleFragShader = tempShader.value();


		vk::PushConstantRange pushConstant = vk::PushConstantRange(
			vk::ShaderStageFlagBits::eVertex,
			0, sizeof(MatrixPushConstant)
		);


		PipelineBuilder pipelineBuilder{ swapchain.extentImage };
		pipelineBuilder.addShader(vk::ShaderStageFlagBits::eVertex, triangleVertexShader);
		pipelineBuilder.addShader(vk::ShaderStageFlagBits::eFragment, triangleFragShader);
		pipelineBuilder.addPushConstant(pushConstant);

		trianglePipeline = pipelineBuilder.build_pipeline(device.get(), renderPass);
		pipelineLayout = pipelineBuilder._pipelineLayout;


		vkDestroyShaderModule(device.get(), triangleVertexShader, nullptr);
		vkDestroyShaderModule(device.get(), triangleFragShader, nullptr);


		triangleMesh = loadobj("../RayTracing_CPU/test/Sponza-master/sponza.obj");

		//we don't care about the vertex normals
		uploadMesh(allocator, triangleMesh);
	}

VulkanContext::~VulkanContext() {
	auto result = device.get().waitForFences(renderFence, true, 1000000000);

	allocator.destroyBuffer(triangleMesh.vertexBuffer);

	device.get().destroyPipeline(trianglePipeline);
	device.get().destroyPipelineLayout(pipelineLayout);
	device.get().destroyRenderPass(renderPass);
	device.get().destroyCommandPool(commandPool);
	device.get().destroySemaphore(presentSemaphore);
	device.get().destroySemaphore(renderSemaphore);
	device.get().destroyFence(renderFence);

	swapchain.destroy(device);
	vmaDestroyAllocator(allocator);

	glfwTerminate();
}

void VulkanContext::draw() {
	auto result = device.get().waitForFences(renderFence, true, 1000000000);
	device.get().resetFences(renderFence);

	auto nextImage = device.get().acquireNextImageKHR(swapchain.swapchain, 1000000000, presentSemaphore);
	uint32_t swapchainImageIndex = nextImage.value;

	vk::CommandBuffer cmd = mainCommandBuffer[0];

	cmd.reset();

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
	cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	std::array<float, 4> clearColor{};
	clearColor[0] = 0.0f;
	clearColor[1] = 0.0f;
	clearColor[2] = 0.0f;
	clearColor[3] = 1.0f;

	vk::ClearValue clearValue = vk::ClearValue(clearColor);
	vk::ClearValue depthClear = vk::ClearValue(1.0f);

	vk::ClearValue clearValues[] = { clearValue, depthClear };

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	vk::RenderPassBeginInfo rpInfo = {};
	rpInfo.renderPass = renderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent = swapchain.extentImage;
	rpInfo.framebuffer = swapchain.framebuffers[swapchainImageIndex];
	rpInfo.setClearValues(clearValues); //connect clear values

	cmd.beginRenderPass(rpInfo, vk::SubpassContents::eInline);

	glm::mat4 projection = glm::perspective(glm::radians(70.f), WIDTH / (float)HEIGHT, 0.1f, 2000.0f);
	projection[1][1] *= -1;

	//model rotation
	glm::mat4 model = glm::lookAt(camera.position, camera.position + camera.direction, glm::vec3(0.f, 1.f, 0.f));

	//calculate final mesh matrix
	glm::mat4 mesh_matrix = projection * model;

	MatrixPushConstant constants = {};
	constants.transformMatrix = mesh_matrix;

	//upload the matrix to the GPU via push constants
	cmd.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(MatrixPushConstant), &constants);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, trianglePipeline);

	//bind the mesh vertex buffer with offset 0
	vk::DeviceSize offset = 0;
	cmd.bindVertexBuffers(0, triangleMesh.vertexBuffer.buffer, offset);

	//we can now draw the mesh
	cmd.draw(triangleMesh.vertices.size(), 1, 0, 0);

	//finalize the render pass
	cmd.endRenderPass();

	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	cmd.end();


	//prepare the submission to the queue.
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	vk::SubmitInfo submit = {};

	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	submit.setWaitDstStageMask(waitStage);
	submit.setWaitSemaphores(presentSemaphore);
	submit.setSignalSemaphores(renderSemaphore);
	submit.setCommandBuffers(cmd);

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	allQueue.submit(submit, renderFence);

	// this will put the image we just rendered into the visible window.
	// we want to wait on the _renderSemaphore for that,
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	vk::PresentInfoKHR presentInfo = {};
	presentInfo.setSwapchains(swapchain.swapchain);
	presentInfo.setWaitSemaphores(renderSemaphore);
	presentInfo.setImageIndices(swapchainImageIndex);

	allQueue.presentKHR(presentInfo);
}

void VulkanContext::setCamera(glm::vec3 position, glm::vec3 direction) {
	camera.position = position;
	camera.direction = direction;
}

