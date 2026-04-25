#include "VulkanContext.h"

#include "loader.h"
#include "input.h"

#include <glm/gtx/transform.hpp>
#include <chrono>


int main() {
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	auto window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	VulkanContext vulkan = VulkanContext(window);
	
	Camera camera = {};
	camera.position = glm::vec3(-600.f, 180.f, -30.f);
	camera.direction = glm::vec3(1.f, 0.f, 0.f);

	glfwSetWindowUserPointer(window, &camera);
	glfwSetKeyCallback(window, key_callback);

	auto frame_start = std::chrono::system_clock::now();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		vulkan.setCamera(camera.position, camera.direction);
		vulkan.draw();
		
		auto delta = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - frame_start).count();
		frame_start = std::chrono::system_clock::now();
		glfwSetWindowTitle(window, ("Vulkan! " + std::to_string(1000000 / delta) + " FPS").c_str());
	}
}