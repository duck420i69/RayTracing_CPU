#pragma once

#include "Material.h"


class Matte : public Material {
public:
	MaterialType getType() const override { return MaterialType::MATTE; }
	SampleResult generateSample(const Ray& ray, const Packet& packet) const override;
	float samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const override;
	void interact(const Ray& ray, const SampleResult& sample, Packet& packet) const override;
};