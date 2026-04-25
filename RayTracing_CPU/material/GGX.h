#pragma once

#include "../integrater/Sampling.h"

#include "../Ray.h"
#include "../rtmath.h"

#include "Material.h"

class GGX : public Material {
public:
	GGX() : m_alpha(0.0f) {}
	GGX(float alpha) : m_alpha(alpha) {}

	MaterialType getType() const override { return MaterialType::GGX; }
	SampleResult generateSample(const Ray& ray, const Packet& packet) const override;
	float samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const override;
	void interact(const Ray& ray, const SampleResult& sample, Packet& packet) const override;
private:
	float m_alpha; // GGX roughness parameter

	// GGX Geometry function
	float ggxG1(const vec3& v, const vec3& h, const vec3& n, float alpha) const;

	// GGX Distribution function
	float ggxD(const vec3& h, const vec3& n, float alpha) const;

	float lambdaGGX(const vec3& v, const vec3& h, const vec3& n, float alpha) const;

	vec4 evalBrdf(const vec3& wi, const vec3& wo, const vec3& normal) const;
};