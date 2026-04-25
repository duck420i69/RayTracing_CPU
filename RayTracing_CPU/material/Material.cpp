#include "../Ray.h"
#include "../integrater/Sampling.h"

#include "Material.h"

#include <cmath>
#include <cstdlib>
#include <Eigen/src/Core/MatrixBase.h>
#include <memory>
#include <utility>


// Phong model
static Color BRDF(const vec3& dir, const vec3& outdir, const vec3& normal, std::shared_ptr<Material> material) {
	vec3 reflected_ray = reflect(dir, normal);
	if (!material) return Color(INVERT_PI, INVERT_PI, INVERT_PI, 1.0f);
	return material->diffuse_color * INVERT_PI + material->specular_color * pow(std::max(reflected_ray.dot(outdir), 0.0f), material->specular_exp);
}


SampleResult Glass::generateSample(const Ray& ray, const Packet& packet) const {
	return SampleResult();
}

float Glass::samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const {
	return 1.0f;
}

void Glass::interact(const Ray& ray, const SampleResult& sample, Packet& packet) const {
	packet.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	if (packet.normal.dot(ray.d) < 0) {
		if (random0to1() < Fresnal(ray.refraction, refraction, dot(packet.normal, ray.d * -1.0f))) {
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
		packet.next_dir = refract(ray.d, packet.normal * -1.0f, refraction / 1.0f);

		if (dot(packet.normal, packet.next_dir) < 0.0f) return;
		
		if (random0to1() < Fresnal(ray.refraction, refraction, dot(packet.normal, packet.next_dir))) {
			packet.next_dir = reflect(ray.d, packet.normal * -1.0f);
		}
		else {
			packet.next_dir = refract(ray.d, packet.normal * -1.0f, refraction / 1.0f);
		}
	}
}


SampleResult Plastic::generateSample(const Ray& ray, const Packet& packet) const {
	return random_cos_weight(packet.normal);
}

float Plastic::samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const {
	return wi.dot(normal) > 0.0f ? wi.dot(normal) * INVERT_PI : 0.0f;
}

void Plastic::interact(const Ray& ray, const SampleResult& sample, Packet& packet) const {
	packet.next_dir = sample.sample;
	vec3 reflect_ray = reflect(ray.d, packet.normal);

	vec4 diffuse = getDiffuse({ packet.u, packet.v });
	vec4 specular = getSpecular({ packet.u, packet.v }) * pow(std::max(reflect_ray.dot(packet.next_dir), 0.0f), specular_exp);

	packet.color = (diffuse + specular) * packet.normal.dot(packet.next_dir);
}


SampleResult Mirror::generateSample(const Ray& ray, const Packet& packet) const {
	return SampleResult();
}

float Mirror::samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const {
	return 1.0f;
}

void Mirror::interact(const Ray& ray, const SampleResult& sample, Packet& packet) const {
	packet.next_dir = reflect(ray.d, packet.normal);
	packet.color = specular_color;
}


SampleResult ThinGlass::generateSample(const Ray& ray, const Packet& packet) const {
	return SampleResult();
}

float ThinGlass::samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const {
	return 1.0f;
}

void ThinGlass::interact(const Ray& ray, const SampleResult& sample, Packet& packet) const {
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

inline float Fresnal(float n1, float n2, float cosTheta) {
	float r0 = (n1 - n2) / (n1 + n2);
	r0 = r0 * r0;
	return r0 + (1.0f - r0) * std::pow(1.0f - cosTheta, 5);
}

bool isDeterministic(const std::shared_ptr<Material>& material) {
	MaterialType type = material->getType();

	switch (type) {
		case MaterialType::GLASS:
		case MaterialType::THIN_GLASS:
		case MaterialType::MIRROR:
			return true;
	}

	return false;
}

inline Color Material::getDiffuse(const vec2& uv) const {
	if (texture_map.size() > 0) {
		return diffuse_color * getTexture(uv);
	}
	return diffuse_color;
}

inline Color Material::getSpecular(const vec2& uv) const {
	Color specular = specular_color;

	if (texture_map.size() > 0) {
		specular = specular * getTexture(uv);
	}

	specular = specular * metalic - Color(1.0f, 1.0f, 1.0f, 1.0f) * (1.0f - metalic);

	return specular;
}
