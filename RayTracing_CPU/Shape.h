#pragma once

#include "Sphere.h"
#include "Mesh.h"
#include "Environment.h"

struct Scene {
	BVHLinear tree;
	std::shared_ptr<Environment> environment;
	std::vector<std::shared_ptr<Hittable>> light_source;
	std::vector<std::shared_ptr<Hittable>> objects;
	std::vector<std::shared_ptr<Material>> materials;
};