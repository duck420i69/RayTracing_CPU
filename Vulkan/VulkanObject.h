#pragma once

#include "vma.hpp"

struct VertexInputDescription {

	std::vector<vk::VertexInputBindingDescription> bindings;
	std::vector<vk::VertexInputAttributeDescription> attributes;

	vk::PipelineVertexInputStateCreateFlags flags = {};
};


struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;

	static VertexInputDescription get_vertex_description();
};


struct Mesh {
    std::vector<Vertex> vertices;
    vma::Buffer vertexBuffer;
	vma::Allocator allocator;

	void destroy() {
		allocator.destroyBuffer(vertexBuffer);
	}
};


struct Camera {
	glm::vec3 position;
	glm::vec3 direction;
};


void uploadMesh(vma::Allocator allocator, Mesh& mesh);