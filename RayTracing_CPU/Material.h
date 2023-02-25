#pragma once

#include "math.h"
#include "Ray.h"
#include "Sampling.h"

#include <string>
#include <vector>


struct Packet;

class Material {
public:
	std::string name;
	vec4 ambient_color;
	vec4 specular_color;
	vec4 diffuse_color;
	float refraction, specular_exp;
	int width, height;
	std::vector<vec4> texture_map;
	vec4 getTexture(const vec2& uv) {
		vec2 texcord = { uv.x * width - 0.5f, uv.y * height - 0.5f };
		return texture_map[static_cast<int>(texcord.x) + static_cast<int>(texcord.y) * width];
	}
	virtual bool isTransparent() = 0;
	virtual SampleResult generateSample(const Ray& ray, const Packet& packet) = 0;
	virtual void interact(const Ray& ray, const SampleResult& sample, Packet& packet) = 0;
};

struct Packet {
	float u, v;
	std::shared_ptr<Material> material;
	vec4 color;
	vec4 normal;
	vec4 last_hit;
	vec4 next_dir;
	uint16_t aabb_check;
	uint16_t shape_check;
};

class Matte : public Material {
public:
	bool isTransparent() { return false; }
	SampleResult generateSample(const Ray& ray, const Packet& packet) override;
	void interact(const Ray& ray, const SampleResult& sample, Packet& packet) override;
};

class Metal : public Material {
public:
	bool isTransparent() { return false; }
	SampleResult generateSample(const Ray& ray, const Packet& packet) override;
	void interact(const Ray& ray, const SampleResult& sample, Packet& packet) override;
};

class Glass : public Material {
public:
	bool isTransparent() { return true; }
	SampleResult generateSample(const Ray& ray, const Packet& packet) override;
	void interact(const Ray& ray, const SampleResult& sample, Packet& packet) override;
};

class Plastic : public Material {
public:
	bool isTransparent() { return false; }
	SampleResult generateSample(const Ray& ray, const Packet& packet) override;
	void interact(const Ray& ray, const SampleResult& sample, Packet& packet) override;
};

class Mirror : public Material {
public:
	bool isTransparent() { return true; }
	SampleResult generateSample(const Ray& ray, const Packet& packet) override;
	void interact(const Ray& ray, const SampleResult& sample, Packet& packet) override;
};



