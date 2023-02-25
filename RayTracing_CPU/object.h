#pragma once

#include <memory>
#include <vector>
#include "Material.h"
#include "accelerator.h"

using texel = vec2;

class Hittable {
public:
	std::shared_ptr<Material> material;
	virtual bool hit(const Ray& ray, Packet& result) const = 0;
	virtual Bound getBound() const = 0;
};