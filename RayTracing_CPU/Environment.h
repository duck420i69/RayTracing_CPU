#pragma once

#include "Material.h"

class Environment {
public:
	std::vector<vec4> env_map;
	virtual SampleResult generateSample(const Packet& packet) = 0;
	virtual void interact(const Ray& ray, Packet& packet) = 0;
};

class DefaultEnvironment : public Environment {
public:
	DefaultEnvironment() : DefaultEnvironment({0.0f, 1.0f, 0.0f, 1.0f}, 0.999f) {}
	DefaultEnvironment(vec4 sun_dir, float cos_theta);

	vec4 sun_direction;
	float cos_theta_max;
	float sun_intensity;
	vec4 sky_color;
	float p_sun;
	SampleResult generateSample(const Packet& packet) override;
	void interact(const Ray& ray, Packet& packet) override;
};