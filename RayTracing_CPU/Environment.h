#pragma once

#include "Material.h"

class Environment {
public:
	virtual SampleResult generateSample(const Packet& packet) = 0;
	virtual float samplePDF(const vec4& wi, const vec4& wo, const vec4& normal) = 0;
	virtual void interact(const Ray& ray, Packet& packet) = 0;
};

class DefaultEnvironment : public Environment {
public:
	DefaultEnvironment() : DefaultEnvironment({0.0f, 1.0f, 0.0f, 1.0f}, 0.999f, 1.0f) {}
	DefaultEnvironment(vec4 sun_dir, float cos_theta, float brightness);

	SampleResult generateSample(const Packet& packet) override;
	float samplePDF(const vec4& wi, const vec4& wo, const vec4& normal) override;
	void interact(const Ray& ray, Packet& packet) override;

	vec4 sun_direction;
	float cos_theta_max;
	float sun_intensity;
	vec4 sky_color;
	float p_sun;
};

class EnvironmentHDR : public Environment {
public:
	EnvironmentHDR(std::vector<vec4>&& image, int w, int h);
	~EnvironmentHDR();
	SampleResult generateSample(const Packet& packet);
	float samplePDF(const vec4& wi, const vec4& wo, const vec4& normal) override;
	void interact(const Ray& ray, Packet& packet);

	int width;
	int height;
	std::vector<vec4> env_map;
	std::vector<std::vector<double>> distribution1d;
	std::vector<float> distribution2d;
	std::vector<float> lowresDistribution;
};
