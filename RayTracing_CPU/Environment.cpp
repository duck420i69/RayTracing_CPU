#include "Environment.h"
#include <iostream>


const vec4 SKY_COLOR = vec4(0.15625f, 0.220703f, 0.384766f, 1.0f);

static const float normalize_angle(float x, float cycle) {
	return x >= cycle ? x - cycle : x;
}

DefaultEnvironment::DefaultEnvironment(vec3 sun_dir, float cos_theta, float brightness) : 
	sun_direction(sun_dir), cos_theta_max(cos_theta), 
	sun_intensity(brightness * 0.312f / (1.0f - cos_theta)), 
	sky_color({ 0.52734375f, 0.8046875f, 2.34796875f, 1.0f }) 
{
	p_sun = (sun_intensity * (1.0f - cos_theta_max)) / (sky_color.norm() * cos_theta_max + sun_intensity * (1.0f - cos_theta_max));
}


EnvironmentType DefaultEnvironment::getType() const {
	return EnvironmentType::DEFAULT;
}

SampleResult DefaultEnvironment::generateSample(const Packet& packet) {
	if (random0to1() < p_sun) {
		auto sample = random_in_cone(cos_theta_max, sun_direction);
		float f = std::max((packet.normal.dot(sample.sample) / pi) * (1.0f - p_sun), 0.0f);
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

float DefaultEnvironment::samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) {
	if (wi.dot(sun_direction) >= cos_theta_max) {
		float f = p_sun / (TwoPi * (1.0f - cos_theta_max));
		float p = wi.dot(normal) > 0.0f ? (1.0f - p_sun) * wi.dot(normal) : 0.0f;
		return f + p;
	}
	else return wi.dot(normal) > 0.0f ? (1.0f - p_sun) * wi.dot(normal) : 0.0f;
}


vec4 DefaultEnvironment::interact(const Ray& ray, const Packet& packet) {
	if (sun_direction.dot(ray.d) > cos_theta_max)
		return { sun_intensity, sun_intensity, sun_intensity, 1.0f };
	else
		return sky_color;
}

void DefaultEnvironment::rotate(float pitch, float yaw) {

}


EnvironmentHDR::EnvironmentHDR(std::vector<vec4>&& image, int w, int h) {
	env_map = image;
	width = w;
	height = h;
	pitch = 0.0f;
	yaw = 0.0f;

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
		for (int j = 0; j < w; j++) {
			distribution1d[i][j] /= distribution1d[h - 1][w];
			env_map[i*w + j].w /= distribution1d[h - 1][w];
		}
		distribution1d[i][w] /= distribution1d[h - 1][w];
	}
}


EnvironmentHDR::~EnvironmentHDR() {}


EnvironmentType EnvironmentHDR::getType() const {
	return EnvironmentType::HDR_2D;
}

SampleResult EnvironmentHDR::generateSample(const Packet& packet) {
	double u = random0to1d();
	float lat_res = pi / (float)height;
	float long_res = TwoPi / (float)width;

	long start = 0, end = height - 1;

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
	
	float theta_min = normalize_angle(h * lat_res + pi - pitch, pi);
	float theta_max = theta_min + lat_res;
	float phi_min = normalize_angle(start * long_res + TwoPi - yaw, TwoPi);
	float phi_max = phi_min + long_res;

	auto result = sphere_texture_sampling(theta_min, theta_max, phi_min, phi_max);
	result.inv_pdf /= env_map[h * width + start].w;

	if (result.sample.norm() > 1.0f + epsilon || result.sample.norm() < 1.0f - epsilon) {
		std::cout << "theta = " << theta_min << " - " << theta_max << std::endl;
		std::cout << "phi = " << phi_min << " - " << phi_max << std::endl;
		std::cout << "sample = " + to_string(result.sample) << std::endl << std::endl;
	}

	return result;
}

float EnvironmentHDR::samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) {
	float phi = atan2f(-wi.z(), -wi.x()) + pi;
	float theta = abs(wi.y()) >= 1.0f ? PiOver2 - PiOver2 * wi.y() : acosf(wi.y());
	long u = static_cast<long>(width * phi / TwoPi);
	long v = static_cast<long>(height * theta / pi - 0.01f);

	theta = normalize_angle(theta + pitch, pi);
	phi = normalize_angle(phi + yaw, TwoPi);

	if (u == width) u -= 1;

	static auto integrate_theta = [&](int theta) {
		return cos(theta / (double)height * pi) - cos((theta + 1) / (double)height * pi);
	};

	if (isnan(integrate_theta(v)) || integrate_theta(v) <= 0.0f || env_map[u + v * width].w < 0.0f) {
		std::cout << "v = " << v << "; u = " << u << std::endl;
		std::cout << "theta = " << theta << "; phi = " << phi << std::endl;
		std::cout << "wi = (" << wi.x() << ", " << wi.y() << ", " << wi.z() << ")" << std::endl;
		std::cout << "Area = " << integrate_theta(v) << std::endl;
		std::cout << "p = " << env_map[u + v * width].w << std::endl;

		throw std::runtime_error("NaN detected!");
	}
	
	return (env_map[u + v * width].w) / (integrate_theta(v) * (TwoPi / (float)width));
}


vec4 EnvironmentHDR::interact(const Ray& ray, const Packet& packet) {
	float phi = atan2f(-ray.d.z(), -ray.d.x()) + pi;
	float theta = acosf(ray.d.y());
	theta = normalize_angle(theta + pitch, pi);
	phi = normalize_angle(phi + yaw, TwoPi);
	float u = width * phi / TwoPi;
	float v = height * theta / pi - 0.01f;
	return env_map[static_cast<int>(u) + static_cast<int>(v) * width];
}

void EnvironmentHDR::rotate(float pitch, float yaw) {
	this->pitch += pitch;
	this->yaw += yaw;

	if (this->pitch >= pi) {
		this->pitch -= pi * (int)(this->pitch / pi);
	}
	if (this->pitch < 0) {
		this->pitch -= pi * (int)(this->pitch / pi - 1.0f);
	}

	if (this->yaw >= TwoPi) {
		this->yaw -= TwoPi * (int)(this->yaw / TwoPi);
	}
	if (this->yaw < 0) {
		this->yaw -= TwoPi * (int)(this->yaw / TwoPi - 1.0f);
	}
}
