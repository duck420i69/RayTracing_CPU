#pragma once

#include "../debug.h"
#include "../Ray.h"
#include "../rtmath.h"

#include "../integrater/Sampling.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>


constexpr float INVERT_PI = 1 / pi;


inline float Fresnal(float n1, float n2, float cosTheta);


struct Packet;


enum class MaterialType : uint8_t {
	MATTE,
	METAL,
	GLASS,
	THIN_GLASS,
	PLASTIC,
	MIRROR,
	GGX
};


class Material {
public:
	std::string name;
	Color ambient_color;
	Color specular_color;
	Color diffuse_color;
	float refraction, metalic, specular_exp;
	int width, height;
	std::vector<Color> texture_map;
	Color getTexture(const vec2& uv) const {
		vec2 texcord = { uv.x * width - 0.5f, uv.y * height - 0.5f };
		return texture_map[static_cast<int>(texcord.x) + static_cast<int>(texcord.y) * width];
	}
	virtual MaterialType getType() const = 0;
	virtual SampleResult generateSample(const Ray& ray, const Packet& packet) const = 0;
	virtual float samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const = 0;
	virtual void interact(const Ray& ray, const SampleResult& sample, Packet& packet) const = 0;
protected:
	Color getDiffuse(const vec2& uv) const;
	Color getSpecular(const vec2& uv) const;
};


struct Packet {
	float u, v;
	std::shared_ptr<Material> material;
	uint16_t aabb_check;
	uint16_t shape_check;
	vec3 normal;
	vec3 last_hit;
	vec3 next_dir;
	Color color;
	StackTrace* debugger;
};


class Glass : public Material {
public:
	MaterialType getType() const override { return MaterialType::GLASS; }
	SampleResult generateSample(const Ray& ray, const Packet& packet) const override;
	float samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const override;
	void interact(const Ray& ray, const SampleResult& sample, Packet& packet) const override;
};

class ThinGlass : public Material {
public:
	MaterialType getType() const override { return MaterialType::THIN_GLASS; }
	SampleResult generateSample(const Ray& ray, const Packet& packet) const override;
	float samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const override;
	void interact(const Ray& ray, const SampleResult& sample, Packet& packet) const override;
};

class Plastic : public Material {
public:
	MaterialType getType() const override { return MaterialType::PLASTIC; }
	SampleResult generateSample(const Ray& ray, const Packet& packet) const override;
	float samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const override;
	void interact(const Ray& ray, const SampleResult& sample, Packet& packet) const override;
};

class Mirror : public Material {
public:
	MaterialType getType() const override { return MaterialType::MIRROR; }
	SampleResult generateSample(const Ray& ray, const Packet& packet) const override;
	float samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const override;
	void interact(const Ray& ray, const SampleResult& sample, Packet& packet) const override;
};


bool isDeterministic(const std::shared_ptr<Material>& material);