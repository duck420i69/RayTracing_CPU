#include "Material.h"
#include "Sampling.h"
#include <iostream>

constexpr float INVERT_PI = 1 / pi;


// Phong model
vec4 BRDF(const vec4& dir, const vec4& outdir, const vec4& normal, std::shared_ptr<Material> material) {
	vec4 reflected_ray = reflect(dir, normal);
	if (!material) return vec4(INVERT_PI, INVERT_PI, INVERT_PI, 1.0f);
	return material->diffuse_color * INVERT_PI + material->specular_color * pow(std::max(reflected_ray.dot(outdir), 0.0f), material->specular_exp);
}

inline float Fresnal(const float& n1, const float& n2, const float& cosTheta) {
	float r0 = (n1 - n2) / (n1 + n2);
	r0 = r0 * r0;
	return r0 + (1 - r0) * std::pow(1.0f - cosTheta, 5);
}

SampleResult Matte::generateSample(const Ray& ray, const Packet& packet) {
	return random_cos_weight(packet.normal);
}

float Matte::samplePDF(const vec4& wi, const vec4& wo, const vec4& normal) {
	return wi.dot(normal) > 0.0f ? wi.dot(normal) * INVERT_PI : 0.0f;
}

void Matte::interact(const Ray& ray, const SampleResult& sample, Packet& packet) {
	packet.next_dir = sample.sample;
	if (texture_map.size() > 0) packet.color = getTexture({ packet.u, packet.v }) * packet.normal.dot(ray.d);
	else packet.color = diffuse_color * packet.normal.dot(packet.next_dir);
}

SampleResult Metal::generateSample(const Ray& ray, const Packet& packet) {
	vec4 reflect_ray = reflect(ray.d, packet.normal);
	float smoothness = pow(0.001f, 1.0f / (specular_exp + 1.0f));
	if (random0to1() < Fresnal(1.0f, specular_exp, packet.normal.dot(ray.d)))
		return random_in_cone(smoothness, reflect_ray);
	else
		return random_cos_weight(packet.normal);
}

float Metal::samplePDF(const vec4& wi, const vec4& wo, const vec4& normal) {
	float smoothness = pow(0.001f, 1.0f / (specular_exp + 1.0f));
	float p = Fresnal(1.0f, specular_exp, normal.dot(wo));
	float cosTheta = std::max(wi.dot(normal), 0.0f);
	if (cosTheta < smoothness)
		return p / (TwoPi * (1.0f - smoothness)) + (1.0f - p) * cosTheta * INVERT_PI;
	else
		return cosTheta;
}

void Metal::interact(const Ray& ray, const SampleResult& sample, Packet& packet) {
	vec4 reflect_ray = reflect(ray.d, packet.normal);
	packet.next_dir = sample.sample;
	if (texture_map.size() > 0) {
		packet.color = getTexture({ packet.u, packet.v }) * pow(std::max(reflect_ray.dot(packet.next_dir), 0.0f), specular_exp) * packet.normal.dot(packet.next_dir);
	}
	else {
		packet.color = specular_color * pow(std::max(reflect_ray.dot(packet.next_dir), 0.0f), specular_exp) * packet.normal.dot(packet.next_dir);
	}
}

SampleResult Glass::generateSample(const Ray& ray, const Packet& packet) {
	return SampleResult();
}

float Glass::samplePDF(const vec4& wi, const vec4& wo, const vec4& normal) {
	return 1.0f;
}

void Glass::interact(const Ray& ray, const SampleResult& sample, Packet& packet) {
	packet.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	if (packet.normal.dot(ray.d) < 0) {
		if (random0to1() < Fresnal(ray.refraction, refraction, packet.normal.dot(ray.d * -1.0f))) {
			packet.next_dir = reflect(ray.d, packet.normal);
		}
		else {
			packet.next_dir = refract(ray.d, packet.normal, 1.0f / refraction);
			ray.refraction = refraction;
			packet.color = specular_color;
		}
	}
	else {
		ray.refraction = 1.0f;
		if (random0to1() < Fresnal(ray.refraction, refraction, packet.normal.dot(ray.d))) {
			packet.next_dir = reflect(ray.d, packet.normal * -1.0f);
		}
		else {
			packet.next_dir = refract(ray.d, packet.normal * -1.0f, refraction / 1.0f);
		}
	}
}

SampleResult Plastic::generateSample(const Ray& ray, const Packet& packet) {
	return random_cos_weight(packet.normal);
}

float Plastic::samplePDF(const vec4& wi, const vec4& wo, const vec4& normal) {
	return wi.dot(normal) > 0.0f ? wi.dot(normal) * INVERT_PI : 0.0f;
}

void Plastic::interact(const Ray& ray, const SampleResult& sample, Packet& packet) {
	packet.next_dir = sample.sample;
	vec4 reflect_ray = reflect(ray.d, packet.normal);
	if (texture_map.size() > 0) {
		packet.color = getTexture({ packet.u, packet.v }) * (diffuse_color + specular_color * pow(std::max(reflect_ray.dot(packet.next_dir), 0.0f), specular_exp)) * packet.normal.dot(packet.next_dir);
	}
	else {
		packet.color = (diffuse_color + specular_color * pow(std::max(reflect_ray.dot(packet.next_dir), 0.0f), specular_exp)) * packet.normal.dot(packet.next_dir);
	}
}

SampleResult Mirror::generateSample(const Ray& ray, const Packet& packet) {
	return SampleResult();
}

float Mirror::samplePDF(const vec4& wi, const vec4& wo, const vec4& normal) {
	return 1.0f;
}

void Mirror::interact(const Ray& ray, const SampleResult& sample, Packet& packet) {
	packet.next_dir = reflect(ray.d, packet.normal);
	packet.color = specular_color;
}

SampleResult ThinGlass::generateSample(const Ray& ray, const Packet& packet) {
	return SampleResult();
}

float ThinGlass::samplePDF(const vec4& wi, const vec4& wo, const vec4& normal) {
	return 1.0f;
}

void ThinGlass::interact(const Ray& ray, const SampleResult& sample, Packet& packet) {
	packet.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	if (packet.normal.dot(ray.d) < 0) {
		if (random0to1() < Fresnal(ray.refraction, refraction, packet.normal.dot(ray.d * -1.0f))) {
			packet.next_dir = reflect(ray.d, packet.normal);
		}
		else {
			packet.next_dir = ray.d;
			packet.color = specular_color;
		}
	}
	else {
		ray.refraction = 1.0f;
		if (random0to1() < Fresnal(ray.refraction, refraction, packet.normal.dot(ray.d))) {
			packet.next_dir = reflect(ray.d, packet.normal * -1.0f);
		}
		else {
			packet.next_dir = ray.d;
			packet.color = specular_color;
		}
	}
}
