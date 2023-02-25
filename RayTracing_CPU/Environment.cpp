#include "Environment.h"
#include <iostream>

DefaultEnvironment::DefaultEnvironment(vec4 sun_dir, float cos_theta) : sun_direction(sun_dir), cos_theta_max(cos_theta), sun_intensity(0.312f / (1.0f - cos_theta)), sky_color({ 0.52734375f, 0.8046875f, 0.94796875f, 1.0f }) {
	p_sun = (sun_intensity * (1.0f - cos_theta_max)) / (sky_color.norm() * cos_theta_max + sun_intensity * (1.0f - cos_theta_max));
	p_sun = .25f;

}


SampleResult DefaultEnvironment::generateSample(const Packet& packet) {
	if (random0to1() < p_sun) {
		auto sample = random_in_cone(cos_theta_max, sun_direction);
		float f = (packet.normal.dot(sample.sample) / pi) * (1.0f - p_sun);
		float p = (1.0f / sample.inv_pdf) * p_sun;
		sample.inv_pdf = 1.0f / (f + p);
		return sample;
	}
	else {
		auto sample = random_cos_weight(packet.normal);
		if (sun_direction.dot(sample.sample) > cos_theta_max) {
			float f = (1.0f / sample.inv_pdf) * (1.0f - p_sun);
			float p = (1.0f / (TwoPi * (1.0f - cos_theta_max))) * p_sun;
			sample.inv_pdf = 1.0f / (f + p);
		}
		else {
			sample.inv_pdf *= 1.0f / (1.0f - p_sun);
		}
		return sample;
	}
}


void DefaultEnvironment::interact(const Ray& ray, Packet& packet) {
	if (sun_direction.dot(ray.d) > cos_theta_max)
		packet.color = { sun_intensity, sun_intensity, sun_intensity, 1.0f };
	else
		packet.color = sky_color;
}
