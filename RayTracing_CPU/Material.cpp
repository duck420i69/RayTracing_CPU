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

void Matte::interact(const Ray& ray, const SampleResult& sample, Packet& packet) {
	packet.next_dir = sample.sample;
	if (texture_map.size() > 0) packet.color = getTexture({ packet.u, packet.v }) * packet.normal.dot(ray.d);
	else packet.color = diffuse_color * packet.normal.dot(packet.next_dir);
}

SampleResult Metal::generateSample(const Ray& ray, const Packet& packet) {
	if (random0to1() * 3 < diffuse_color.norm()) return random_cos_weight(packet.normal);
	vec4 reflect_ray = reflect(ray.d, packet.normal);
	float smoothness = 1 - 1 / (specular_exp * specular_exp);
	return random_in_cone(smoothness, reflect_ray);
}

void Metal::interact(const Ray& ray, const SampleResult& sample, Packet& packet) {
	vec4 reflect_ray = reflect(ray.d, packet.normal);
	float smoothness = 1 - 1 / (specular_exp * specular_exp);
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

void Glass::interact(const Ray& ray, const SampleResult& sample, Packet& packet) {
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

void Plastic::interact(const Ray& ray, const SampleResult& sample, Packet& packet) {
	packet.next_dir = sample.sample;
	vec4 reflect_ray = reflect(ray.d, packet.normal);
	if (texture_map.size() > 0) {
		packet.color = getTexture({ packet.u, packet.v }) * (diffuse_color + specular_color * pow(std::max(reflect_ray.dot(packet.next_dir), 0.0f), specular_exp)) * packet.normal.dot(packet.next_dir);
	}
	else {
		packet.color = diffuse_color + specular_color * pow(std::max(reflect_ray.dot(packet.next_dir), 0.0f), specular_exp) * packet.normal.dot(packet.next_dir);
	}
}

SampleResult Mirror::generateSample(const Ray& ray, const Packet& packet) {
	return SampleResult();
}

void Mirror::interact(const Ray& ray, const SampleResult& sample, Packet& packet) {
	packet.next_dir = reflect(ray.d, packet.normal);
	packet.color = specular_color;
}
