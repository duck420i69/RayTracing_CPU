#pragma once

#include "math.h"


constexpr float PiOver2 = pi / 2;
constexpr float PiOver4 = pi / 4;
constexpr float TwoPi = pi * 2;
constexpr float FourPi = pi * 4;

constexpr uint8_t ENVIRONMENT_SAMPLE = 16;
constexpr uint8_t LIGHT_SAMPLE = 16;
constexpr uint8_t BRDF_SAMPLE = 16;

struct SampleResult {
	vec4 sample;
	float inv_pdf;
};

inline float random0to1() {
	static thread_local std::uniform_real_distribution<float> distribution(0.0, 1.0);
	static thread_local std::mt19937 generator;
	return distribution(generator);
}

inline SampleResult random_in_sphere() {
	float z = 1.0f - 2.0f * random0to1();
	float r = std::sqrt(1 - z * z);
	float phi = 2.0f * pi * random0to1();
	SampleResult result;
	result.sample = vec4(r * std::cos(phi), r * std::sin(phi), z, 1.0f);
	result.inv_pdf = FourPi;
	return result;
}

inline SampleResult random_in_cone(const float& cosThetaMax, const vec4& direction) {
	float u = random0to1();
	float cosTheta = 1.0f - u + u * cosThetaMax;
	float sinTheta = std::sqrt(1 - cosTheta * cosTheta);
	float phi = TwoPi * random0to1();
	vec4 random_direction(sinTheta * std::cos(phi), cosTheta, sinTheta * std::sin(phi), 1.0f);
	SampleResult result;
	result.inv_pdf = TwoPi * (1.0f - cosThetaMax);
	if (direction.v(1) == 1.0f) {
		result.sample = random_direction;
		return result;
	}
	vec4 x = direction.cross(vec4(0.0f, 1.0f, 0.0f, 1.0f));
	vec4 z = direction.cross(x);
	x.normalize();
	z.normalize();
	result.sample = x * random_direction.v(0) + direction * random_direction.v(1) + z * random_direction.v(2);
	return result;
}

inline vec2 random_in_disk() {
	float u1 = random0to1() * 2.0f - 1.0f;
	float u2 = random0to1() * 2.0f - 1.0f;

	if (u1 == 0.0f && u2 == 0.0f) return { 0.0f, 0.0f };

	float theta, r;
	if (std::abs(u1) > std::abs(u2)) {
		r = u1;
		theta = PiOver4 * (u2 / u1);
	}
	else {
		r = u2;
		theta = PiOver2 - PiOver4 * (u1 / u2);
	}
	return { std::cos(theta) * r, std::sin(theta) * r };
}

inline SampleResult random_cos_weight(const vec4& normal) {
	vec2 disk;
	do { disk = random_in_disk(); } while (disk.x * disk.x + disk.y * disk.y >= 1.0f);
	SampleResult result;
	result.sample = { disk.x, std::sqrt(std::max(0.0f, 1.0f - disk.x * disk.x - disk.y * disk.y)), disk.y, 1.0f };
	result.inv_pdf = pi / result.sample.v(1);
	if (normal.v(1) == 1.0f) return result;
	vec4 x = normal.cross(vec4(0.0f, 1.0f, 0.0f, 1.0f));
	vec4 z = normal.cross(x);
	x.normalize();
	z.normalize();
	result.sample = x * result.sample.v(0) + normal * result.sample.v(1) + z * result.sample.v(2);
	return result;
}


class Reservoir {
public:
	float total_weight;
	float sample_weight;
	SampleResult sample;
	uint16_t M;

	Reservoir() : total_weight(0.0), sample_weight(0.0), M(0.0) {}
	void addSample(const SampleResult& sample, float weight);
	void combineReservoir(const Reservoir& other);
};