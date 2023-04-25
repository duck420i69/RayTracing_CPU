#include "Environment.h"
#include <iostream>


const vec4 SKY_COLOR = vec4(0.52734375f, 0.8046875f, 0.94796875f, 1.0f);

DefaultEnvironment::DefaultEnvironment(vec4 sun_dir, float cos_theta, float brightness) : sun_direction(sun_dir), cos_theta_max(cos_theta), sun_intensity(brightness * 0.312f / (1.0f - cos_theta)), sky_color({ 0.52734375f, 0.8046875f, 2.34796875f, 1.0f }) {
	p_sun = (sun_intensity * (1.0f - cos_theta_max)) / (sky_color.norm() * cos_theta_max + sun_intensity * (1.0f - cos_theta_max));
	p_sun = .25f;
}


SampleResult DefaultEnvironment::generateSample(const Packet& packet) {
	if (random0to1() < p_sun) {
		auto sample = random_in_cone(cos_theta_max, sun_direction);
		float f = packet.normal.dot(sample.sample) > 0.0f ? (packet.normal.dot(sample.sample) / pi) * (1.0f - p_sun) : 0.0f;
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

float DefaultEnvironment::samplePDF(const vec4& wi, const vec4& wo, const vec4& normal) {
	if (wi.dot(sun_direction) >= cos_theta_max) {
		float f = p_sun / (TwoPi * (1.0f - cos_theta_max));
		float p = wi.dot(normal) > 0.0f ? (1.0f - p_sun) * wi.dot(normal) : 0.0f;
		return f + p;
	}
	else return wi.dot(normal) > 0.0f ? (1.0f - p_sun) * wi.dot(normal) : 0.0f;
}


void DefaultEnvironment::interact(const Ray& ray, Packet& packet) {
	if (sun_direction.dot(ray.d) > cos_theta_max)
		packet.color = { sun_intensity, sun_intensity, sun_intensity, 1.0f };
	else
		packet.color = sky_color;
}


EnvironmentHDR::EnvironmentHDR(std::vector<vec4>&& image, int w, int h) {
	env_map = image;
	width = w;
	height = h;

	distribution1d.resize(h);

	distribution1d[0].resize(w + 1);
	distribution1d[0][0] = 0.0f;
	for (int j = 0; j < w; j++) {
		distribution1d[0][j + 1] = distribution1d[0][j] + image[j].w;
	}

	for (int i = 1; i < h; i++) {
		distribution1d[i].resize(w + 1);
		distribution1d[i][0] = distribution1d[i - 1][w];
		for (int j = 0; j < w; j++) {
			distribution1d[i][j + 1] = distribution1d[i][j] + image[i * w + j].w;
		}
	}

	for (int i = 0; i < h; i++) {
		for (int j = 0; j <= w; j++) {
			distribution1d[i][j] /= distribution1d[h - 1][w];
			env_map[i*w + j].w /= distribution1d[h - 1][w];
		}
	}
}


EnvironmentHDR::~EnvironmentHDR() {}


SampleResult EnvironmentHDR::generateSample(const Packet& packet) {
	double u = random0to1d();
	long start = 0, end = height - 1;
	if (u >= distribution1d[height - 1][0]) {
		int h = height - 1;
		start = 0, end = width;
		while (start != end - 1) {
			if (u < distribution1d[h][(start + end) / 2]) {
				end = (start + end) / 2;
			}
			else {
				start = (start + end) / 2;
			}
		}
		auto result = sphere_texture_sampling(h / (float)height * pi, (h + 1) / (float)height * pi, start / (float)width * TwoPi, (start + 1) / (float)width * TwoPi);
		result.inv_pdf /= env_map[h*width + start].w;
		return result;
	}
	while (start != end - 1) {
		if (u < distribution1d[(start + end) / 2][0]) {
			end = (start + end) / 2;
		}
		else {
			start = (start + end) / 2;
		}
	}
	int h = start;
	start = 0, end = width;
	while (start != end - 1) {
		if (u < distribution1d[h][(start + end) / 2]) {
			end = (start + end) / 2;
		}
		else {
			start = (start + end) / 2;
		}
	}
	auto result = sphere_texture_sampling(h / (float)height * pi, (h + 1) / (float)height * pi, start / (float)width * TwoPi, (start + 1) / (float)width * TwoPi);
	result.inv_pdf /= env_map[h * width + start].w;

	return result;
}

float EnvironmentHDR::samplePDF(const vec4& wi, const vec4& wo, const vec4& normal) {
	float phi = atan2f(-wi.v(2), -wi.v(0)) + pi;
	float theta = acosf(wi.v(1));
	long u = static_cast<long>(width * phi / TwoPi);
	long v = static_cast<long>(height * theta / pi - 0.01f);
	static auto integrate_theta = [&] (int theta) {
		return cos(theta / (double)height * pi) - cos((theta + 1) / (double)height * pi);
	};
	if (u == width) u -= 1;
	if (isnan(integrate_theta(v)) || integrate_theta(v) <= 0.0f || env_map[u + v * width].w < 0.0f) {
		std::cout << v << " " << u << std::endl;
		std::cout << theta << " " << phi << std::endl;
		std::cout << wi.v(0) << " " << wi.v(1) << " " << wi.v(2) << std::endl;
		std::cout << integrate_theta(v) << std::endl;
		std::cout << env_map[u + v * width].w << std::endl;

		throw std::runtime_error("NaN detected!");
	}
	return (env_map[u + v * width].w) / (integrate_theta(v) * (TwoPi / (float)width));
}


void EnvironmentHDR::interact(const Ray& ray, Packet& packet) {
	float phi = atan2f(-ray.d.v.z(), -ray.d.v.x()) + pi;
	float theta = acosf(ray.d.v.y());
	float u = width * phi / TwoPi;
	float v = height * theta / pi - 0.01f;
	packet.color = env_map[static_cast<int>(u) + static_cast<int>(v) * width];
}
