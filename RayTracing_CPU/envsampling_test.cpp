#include "threading.h"
#include "loader.h"
#include "input.h"

#include "GLFW/GLFW3.h"
#include <cstdlib>

/*
int main() {
	auto environment = loadEnvHDR("skybox/syferfontein_6d_clear_puresky_4k.hdr");
	float epsilon = 0.0000001f;
	float surface_area = (cos(0.5f * pi) - cos((0.5f + 1 / (float)environment->height) * pi)) / environment->width * TwoPi;
	vec4 dummy_vec = { 0,0,0,0 };
	vec4 random_direction(1.0f, 0.0f, 0.0f, 1.0f);
	Packet packet;
	environment->interact(Ray(dummy_vec, random_direction), packet);
	float ratio = packet.color.norm() * surface_area / environment->samplePDF(random_direction, dummy_vec, dummy_vec);
	vec4 color = { 0,0,0,0 };


	for (int i = 0; i < environment->height; i++) {
		for (int j = 0; j < environment->width; j++) {
			
			surface_area = (cos(i / environment->width / (float)environment->height * pi) - cos((i / environment->width + 1) / (float)environment->height * pi)) / environment->width * TwoPi;
			float cosTheta = cos((i / (float)environment->height) * pi);
			float sinTheta = std::sqrt(1 - cosTheta * cosTheta);
			float phi = j / (float)environment->width * TwoPi;
			random_direction = vec4(sinTheta * std::cos(phi), cosTheta, sinTheta * std::sin(phi), 1.0f);
			
			environment->interact(Ray(dummy_vec, random_direction), packet);
			if (packet.color.norm() * surface_area - ratio * environment->samplePDF(random_direction, dummy_vec, dummy_vec) > epsilon) {
				std::cout << packet.color.norm() * surface_area << " " << environment->samplePDF(random_direction, dummy_vec, dummy_vec) << std::endl;
			}

			color = color + (packet.color * (1.0f / environment->samplePDF(random_direction, dummy_vec, dummy_vec)) * std::max(random_direction.dot({0.0f, 1.0f, 0.0f, 1.0f}), 0.0f));
		}
	}
	color = color * (1.0f / (environment->height * environment->width));
	std::cout << color.v(0) << " " << color.v(1) << " " << color.v(2) << "\n";

	color = { 0,0,0,0 };

	for (long i = 0; i < 1000000; i++) {
		auto sample = random_cos_weight({ 0.0, 1.0, 0.0, 1.0 });
		random_direction = sample.sample;

		environment->interact(Ray(dummy_vec, random_direction), packet);

		color = color + packet.color * sample.inv_pdf * std::max(random_direction.dot({ 0.0f, 1.0f, 0.0f, 1.0f }), 0.0f);
	}
	color = color * (1.0 / 1000000.0);
	std::cout << color.v(0) << " " << color.v(1) << " " << color.v(2) << "\n";

	color = { 0,0,0,0 };

	for (long i = 0; i < 1000000; i++) {
		auto sample = random_on_sphere();
		random_direction = sample.sample;

		environment->interact(Ray(dummy_vec, random_direction), packet);

		color = color + packet.color * sample.inv_pdf * std::max(random_direction.dot({ 0.0f, 1.0f, 0.0f, 1.0f }), 0.0f);
	}
	color = color * (1.0 / 1000000.0);
	std::cout << color.v(0) << " " << color.v(1) << " " << color.v(2) << "\n";

	color = { 0,0,0,0 };

	for (long i = 0; i < 1000000; i++) {
		auto sample = environment->generateSample(packet);
		random_direction = sample.sample;

		environment->interact(Ray(dummy_vec, random_direction), packet);

		color = color + packet.color * sample.inv_pdf * std::max(random_direction.dot({ 0.0f, 1.0f, 0.0f, 1.0f }), 0.0f);
	}
	color = color * (1.0 / 1000000.0);
	std::cout << color.v(0) << " " << color.v(1) << " " << color.v(2) << "\n";
}
*/