#pragma once

#include "Camera.h"
#include "Environment.h"
#include "object/Mesh.h"
#include "object/Sphere.h"


struct Scene {
	BVHLinear tree;
	Camera camera;
	std::shared_ptr<Environment> environment;
	std::vector<std::shared_ptr<Hittable>> light_source;
	std::vector<std::shared_ptr<Hittable>> objects;
	std::vector<std::shared_ptr<Material>> materials;

	void buildTree();
};