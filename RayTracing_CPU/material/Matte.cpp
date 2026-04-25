#include "Matte.h"


SampleResult Matte::generateSample(const Ray& ray, const Packet& packet) const {
	return random_cos_weight(packet.normal);
}

float Matte::samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const {
	return wi.dot(normal) > 0.0f ? wi.dot(normal) * INVERT_PI : 0.0f;
}

void Matte::interact(const Ray& ray, const SampleResult& sample, Packet& packet) const {
	packet.next_dir = sample.sample;
	if (texture_map.size() > 0) packet.color = getTexture({ packet.u, packet.v }) * packet.normal.dot(ray.d);
	else packet.color = diffuse_color * packet.normal.dot(packet.next_dir);
}