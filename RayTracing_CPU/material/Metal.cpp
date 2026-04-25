#include "Metal.h"


SampleResult Metal::generateSample(const Ray& ray, const Packet& packet) const {
	vec3 reflect_ray = reflect(ray.d, packet.normal);
	float smoothness = pow(0.001f, 1.0f / (specular_exp + 1.0f));
	if (random0to1() < Fresnal(1.0f, specular_exp, packet.normal.dot(ray.d)))
		return random_in_cone(smoothness, reflect_ray);
	else
		return random_cos_weight(packet.normal);
}

float Metal::samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const {
	float smoothness = pow(0.001f, 1.0f / (specular_exp + 1.0f));
	float p = Fresnal(1.0f, specular_exp, normal.dot(wo));
	float cosTheta = std::max(wi.dot(normal), 0.0f);
	if (cosTheta < smoothness)
		return p / (TwoPi * (1.0f - smoothness)) + (1.0f - p) * cosTheta * INVERT_PI;
	else
		return cosTheta;
}

void Metal::interact(const Ray& ray, const SampleResult& sample, Packet& packet) const {
	vec3 reflect_ray = reflect(ray.d, packet.normal);
	packet.next_dir = sample.sample;
	if (texture_map.size() > 0) {
		packet.color = getTexture({ packet.u, packet.v }) * pow(std::max(reflect_ray.dot(packet.next_dir), 0.0f), specular_exp) * packet.normal.dot(packet.next_dir);
	}
	else {
		packet.color = specular_color * pow(std::max(reflect_ray.dot(packet.next_dir), 0.0f), specular_exp) * packet.normal.dot(packet.next_dir);
	}
}