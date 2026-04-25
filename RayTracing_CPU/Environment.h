#pragma once

#include "integrater/Sampling.h"
#include "material/Material.h"

#include "Ray.h"

#include <vector>


enum class EnvironmentType : uint8_t {
	DEFAULT,
	HDR_2D,
};


class Environment {
public:
	virtual EnvironmentType getType() const = 0;
	virtual SampleResult generateSample(const Packet& packet) = 0;
	virtual float samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) = 0;
	virtual vec4 interact(const Ray& ray, const Packet& packet) = 0;
	virtual void rotate(float pitch, float yaw) = 0;
};

class DefaultEnvironment : public Environment {
public:
	DefaultEnvironment() : DefaultEnvironment({ 0.0f, 1.0f, 0.0f }, 0.999f, 1.0f) {}
	DefaultEnvironment(vec3 sun_dir, float cos_theta, float brightness);

	EnvironmentType getType() const override;
	SampleResult generateSample(const Packet& packet) override;
	float samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) override;
	vec4 interact(const Ray& ray, const Packet& packet) override;
	void rotate(float pitch, float yaw);


	float cos_theta_max;
	float sun_intensity;

	float p_sun;

	vec3 sun_direction;
	Color sky_color;
};

class EnvironmentHDR : public Environment {
public:
	EnvironmentHDR(std::vector<vec4>&& image, int w, int h);
	~EnvironmentHDR();

	EnvironmentType getType() const override;
	SampleResult generateSample(const Packet& packet);
	float samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) override;
	vec4 interact(const Ray& ray, const Packet& packet);
	void rotate(float pitch, float yaw);

	float pitch;
	float yaw;
	int width;
	int height;

	std::vector<vec4> env_map;
	std::vector<std::vector<double>> distribution1d;
	std::vector<float> distribution2d;
	std::vector<float> lowresDistribution;
};
