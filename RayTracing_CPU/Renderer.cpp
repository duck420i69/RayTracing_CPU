#include "Renderer.h"
#include <iostream>


constexpr float NEXT_RAY_SAMPLING_PROPABILITY = 0.9f;

constexpr uint8_t ENVIRONMENT_SAMPLE = 4;
constexpr uint8_t LIGHT_SAMPLE = 2;
constexpr uint8_t BRDF_SAMPLE = 4;


SampleResult RIS(const Scene& scene, const Ray& ray, Packet& packet, bool debug = false) {
	Reservoir r;
	Packet tempPacket{};

	for (int i = 0; i < ENVIRONMENT_SAMPLE; i++) {
		auto sample = scene.environment->generateSample(packet);
		if (sample.sample.dot(packet.normal) < 0) {
			r.M++;
		}
		else {
			sample.inv_pdf = ENVIRONMENT_SAMPLE / (ENVIRONMENT_SAMPLE / sample.inv_pdf + BRDF_SAMPLE * packet.material->samplePDF(sample.sample, ray.d, packet.normal));
			packet.material->interact(ray, sample, packet);
			scene.environment->interact(Ray(packet.last_hit, sample.sample), tempPacket);
			packet.color = tempPacket.color * packet.color * sample.inv_pdf;
			if (debug) {
				std::cout << "Propose: " << sample.sample.v(0) << ", " << sample.sample.v(1) << ", " << sample.sample.v(2) << "\n";
				std::cout << "Invert pdf: " << sample.inv_pdf << " - weight: " << packet.color.norm() << "\n\n";
			}
			r.addSample(sample, packet.color.norm());
		}
	}

	for (int i = 0; i < BRDF_SAMPLE; i++) {
		auto sample = packet.material->generateSample(ray, packet);
		if (sample.sample.dot(packet.normal) < 0) {
			r.M++;
		}
		else {
			if (sample.sample.v(1) >= 1.0f) {
				sample.sample.normalize();
			}
			if (sample.sample.v(1) <= -1.0f) {
				sample.sample.normalize();
			}
			packet.material->interact(ray, sample, packet);
			scene.environment->interact(Ray(packet.last_hit, sample.sample), tempPacket);

			sample.inv_pdf = BRDF_SAMPLE / (ENVIRONMENT_SAMPLE * scene.environment->samplePDF(sample.sample, ray.d, packet.normal) + BRDF_SAMPLE / sample.inv_pdf);
			packet.color = tempPacket.color * packet.color * sample.inv_pdf;
			r.addSample(sample, packet.color.norm());
		}
	}

	SampleResult final_sample;
	final_sample = r.sample;
	if (r.sample_weight > 0.00001f) final_sample.inv_pdf = r.sample.inv_pdf * r.total_weight / (r.sample_weight * r.M);
	else final_sample = random_cos_weight(packet.normal);

	if (isnan(final_sample.inv_pdf)) {
		std::cout << r.sample.inv_pdf << "\n";
		std::cout << r.total_weight << "\n";
		std::cout << r.sample_weight << "\n";

		throw std::runtime_error("NaN detected!");
	}

	return final_sample;
}


bool find_hit(const Scene& objects, const Ray& ray, Packet& result) {
	vec4 invertRayDir = { 1 / ray.d.v(0), 1 / ray.d.v(1), 1 / ray.d.v(2), 1.0f };
	uint32_t iter = 0, stack_size = 0;
	uint32_t stack[64];

	auto& bvh = objects.tree.bvh;

	bool hitted = false;

	while (true) {
		result.aabb_check++;
		if (bvh[iter].bound.hit(ray.o, invertRayDir, ray.tMax)) {
			if (bvh[iter].next_index == 0) {
				auto& obj = objects.objects[bvh[iter].obj_index];
				result.shape_check++;
				if (obj->hit(ray, result)) hitted = true;
				if (stack_size < 1) break;
				iter = stack[--stack_size];
			}
			else {
				if (invertRayDir.v(0) > 0) {
					stack[stack_size++] = bvh[iter++].next_index;
				}
				else {
					stack[stack_size++] = iter + 1;
					iter = bvh[iter].next_index;
				}
			}
		}
		else {
			if (stack_size < 1) break;
			iter = stack[--stack_size];
		}
	}

	return hitted;
}

PixelData trace(const Scene& scene, const Ray& ray, const int& depth, bool debug) {
	vec4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec4 iter_src = ray.o;
	vec4 iter_dir = ray.d;

	Packet packet{};

	SampleResult sample;

	int ray_count = 0;

	for (int i = 0; i < depth; i++) {
		Ray next_ray = Ray(iter_src, iter_dir);

		if (ray_count >= 20) {
			PixelData result;
			result.boundHit = packet.aabb_check;
			result.shapeHit = packet.shape_check;
			result.color = { 0.0f, 0.0f, 0.0f, 1.0f };
			return result;
		}

		if (debug) std::cout << "Bounce: " << i + 1 << "\n\n";

		if (!find_hit(scene, next_ray, packet)) {
			if (debug) std::cout << "Hit the sky! " << "\n\n";
			scene.environment->interact(next_ray, packet);
			col = col * packet.color;
			PixelData result;
			result.boundHit = packet.aabb_check;
			result.shapeHit = packet.shape_check;
			result.color = col;
			return result;
		}
		else {
			if (!packet.material->isTransparent()) {
				sample = RIS(scene, ray, packet, false);
				packet.material->interact(next_ray, sample, packet);
				ray_count++;
			}
			else {
				sample.inv_pdf = 1.0f;
				packet.material->interact(next_ray, sample, packet);
				ray_count++;
				i--;
			}

			col = col * packet.color * sample.inv_pdf;
	
			if (debug) {
				std::cout << "Hit pos: " << packet.last_hit.v(0) << ", " << packet.last_hit.v(1) << ", " << packet.last_hit.v(2) << "\n";
				std::cout << "Material hitted: " << packet.material->name << "\n";
				std::cout << "Incoming ray: " << iter_dir.v(0) << ", " << iter_dir.v(1) << ", " << iter_dir.v(2) << "\n";
				std::cout << "Normal: " << packet.normal.v(0) << ", " << packet.normal.v(1) << ", " << packet.normal.v(2) << "\n";
				std::cout << "Next ray: " << packet.next_dir.v(0) << ", " << packet.next_dir.v(1) << ", " << packet.next_dir.v(2) << "\n";
				std::cout << "Dot normal: " << packet.normal.dot(packet.next_dir) << "\n";
				std::cout << "Invert pdf: " << sample.inv_pdf << "\n";;
				std::cout << "Current ray color: " << col.v(0) << " " << col.v(1) << " " << col.v(2) << "\n\n";
			}

			if (packet.normal.dot(packet.next_dir) > 0.0001f) iter_src = packet.last_hit + packet.normal * 0.001f;
			else if (packet.normal.dot(packet.next_dir) < -0.0001f) iter_src = packet.last_hit + packet.normal * -0.001f;

			iter_dir = packet.next_dir;

			if (isnan(col.norm()) || col.v(0) < 0.0f || col.v(1) < 0.0f || col.v(2) < 0.0f) {
				std::cout << packet.color.norm() << "\n";
				std::cout << packet.color.v(0) << " " << packet.color.v(1) << " " << packet.color.v(2) << "\n";
				std::cout << sample.inv_pdf << "\n";
				std::cout << packet.normal.dot(packet.next_dir) << "\n";

				throw std::runtime_error("NaN detected!");
			}
		}
	}

	PixelData result;
	result.boundHit = packet.aabb_check;
	result.shapeHit = packet.shape_check;
	result.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	return result;
}

PixelData trace_direct(const Scene& scene, const Ray& ray, const int& depth, bool debug) {
	vec4 iter_src = ray.o;
	vec4 iter_dir = ray.d;

	SampleResult sample;

	Packet packet{};
	Ray next_ray = Ray(iter_src, iter_dir);
	if (!find_hit(scene, next_ray, packet)) {
		scene.environment->interact(next_ray, packet);
		PixelData result;
		result.boundHit = packet.aabb_check;
		result.shapeHit = packet.shape_check;
		result.color = packet.color;
		return result;
	}
	else {
		if (!packet.material->isTransparent()) {
			sample.sample = packet.normal;
			packet.material->interact(next_ray, sample, packet);
		}
		else {
			sample.inv_pdf = 1.0f;
			packet.material->interact(next_ray, sample, packet);
		}

		vec4 color = packet.color;
		color = color * (0.5f + 0.5f * packet.normal.dot(vec4(0.0f, 1.0f, 0.0f, 1.0f)));

		PixelData result;
		result.boundHit = packet.aabb_check;
		result.shapeHit = packet.shape_check;
		result.color = color;
		return result;
	}
}