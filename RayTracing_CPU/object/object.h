#pragma once

#include "../accelerator/accelerator.h"
#include "../material/Material.h"

#include <memory>
#include <vector>


enum class HittableType : uint8_t {
	MESH,
	SPHERE,
};


class Hittable {
public:
	std::shared_ptr<Material> material;
	virtual HittableType getType() const = 0;
	virtual bool hit(const Ray& ray, Packet& result) const = 0;
	virtual Bound getBound() const = 0;
	virtual float getCost() const = 0;
};