#include "../debug.h"
#include "../config.h"

#include "Renderer.h"

#include <iostream>


SampleResult MIS(const Scene& scene, const Ray& ray, Packet& packet) {
	SampleResult sample{};
	float brdf_pdf = 0.0f;
	float env_pdf = 0.0f;

	if (random0to1() < 0.5f) {
		do {
			sample = scene.environment->generateSample(packet);
		} while (dot(sample.sample, packet.normal) < epsilon);
		env_pdf = 1.0f / sample.inv_pdf;
		brdf_pdf = packet.material->samplePDF(sample.sample, ray.d, packet.normal);
	}
	else {
		sample = packet.material->generateSample(ray, packet);
		brdf_pdf = 1.0f / sample.inv_pdf;
		env_pdf = scene.environment->samplePDF(sample.sample, ray.d, packet.normal);
	}

	sample.inv_pdf = 1.0f / (0.5f * brdf_pdf + 0.5f * env_pdf);


	if (sample.sample.norm() > 1.0f + epsilon || sample.sample.norm() < 1.0f - epsilon) {
		packet.debugger->LOG_STR("sample.sample", to_string(sample.sample));

		throw packet.debugger->throwException("Sample is not normalized (Environment)");
	}
	if (isinf(env_pdf) || env_pdf <= 0 || isinf(brdf_pdf) || brdf_pdf < 0) {
		packet.debugger->LOG_STR("sample.sample", to_string(sample.sample));
		packet.debugger->LOG_VAR(brdf_pdf);
		packet.debugger->LOG_VAR(env_pdf);
		throw packet.debugger->throwException("Abnormal PDF");
	}

	return sample;
}

#if 0
SampleResult RIS(const Scene& scene, const Ray& ray, Packet& packet) {
	Reservoir r;
	vec4 env_color;

	for (int i = 0; i < ENVIRONMENT_SAMPLE; i++) {
		auto sample = scene.environment->generateSample(packet);
		if (sample.sample.dot(packet.normal) < epsilon) {
			r.M++;
		}
		else {
			packet.material->interact(ray, sample, packet);
			env_color = scene.environment->interact(Ray(packet.last_hit, sample.sample), packet);

			float env_pdf = 1.0f / sample.inv_pdf;
			float brdf_pdf = packet.material->samplePDF(sample.sample, ray.d, packet.normal);
			
			sample.inv_pdf = ENVIRONMENT_SAMPLE / (ENVIRONMENT_SAMPLE * env_pdf + BRDF_SAMPLE * brdf_pdf);
			packet.color = env_color * packet.color * sample.inv_pdf;
			r.addSample(sample, packet.color.norm());

			if (isinf(sample.inv_pdf) || sample.inv_pdf <= 0 || isinf(brdf_pdf) || brdf_pdf < 0) {
				packet.debugger->LOG("cos", packet.normal.dot(r.sample.sample));
				packet.debugger->LOG_STR("sample.sample", to_string(sample.sample));
				packet.debugger->LOG_VAR(ENVIRONMENT_SAMPLE * brdf_pdf);
				packet.debugger->LOG_VAR(BRDF_SAMPLE * env_pdf);
				std::cout << packet.debugger->toString();
				throw packet.debugger->throwException("abnormal pdf");
			}
		}
	}

	for (int i = 0; i < BRDF_SAMPLE; i++) {
		auto sample = packet.material->generateSample(ray, packet);
		if (sample.sample.dot(packet.normal) < epsilon) {
			r.M++;
		}
		else {
			packet.material->interact(ray, sample, packet);
			env_color = scene.environment->interact(Ray(packet.last_hit, sample.sample), packet);
			
			float env_pdf = scene.environment->samplePDF(sample.sample, ray.d, packet.normal);
			float brdf_pdf = 1.0f / sample.inv_pdf;

			sample.inv_pdf = BRDF_SAMPLE * brdf_pdf / (ENVIRONMENT_SAMPLE * env_pdf + BRDF_SAMPLE * brdf_pdf);
			packet.color = env_color * packet.color * sample.inv_pdf;
			r.addSample(sample, packet.color.norm());

			if (isinf(sample.inv_pdf) || sample.inv_pdf <= 0 || isinf(env_pdf) || env_pdf < 0) {
				packet.debugger->LOG("cos", packet.normal.dot(r.sample.sample));
				packet.debugger->LOG_STR("sample.sample", to_string(sample.sample));
				packet.debugger->LOG_VAR(env_pdf);
				packet.debugger->LOG_VAR(brdf_pdf);
				packet.debugger->LOG_VAR(ENVIRONMENT_SAMPLE * env_pdf);
				packet.debugger->LOG_VAR(BRDF_SAMPLE * brdf_pdf);
				std::cout << packet.debugger->toString();
				throw packet.debugger->throwException("abnormal pdf");
			}
		}
	}

	SampleResult final_sample;
	final_sample = r.sample;
	if (r.sample_weight > epsilon) final_sample.inv_pdf = r.sample.inv_pdf * r.total_weight / (r.sample_weight * r.M);
	else final_sample = random_cos_weight(packet.normal);

	return final_sample;
}
#else
SampleResult RIS(const Scene& scene, const Ray& ray, Packet& packet) {
	return MIS(scene, ray, packet);
}
#endif

bool find_hit(const Scene& objects, const Ray& ray, Packet& result) {
	vec3 invertRayDir = { 1 / ray.d.x(), 1 / ray.d.y(), 1 / ray.d.z() };
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
				if (invertRayDir.x() > 0) {
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
	Color col = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec3 iter_src = ray.o;
	vec3 iter_dir = ray.d;

	RayStackTrace stackTracker;
	Packet packet{};
	packet.debugger = (StackTrace*) &stackTracker;

	SampleResult sample;

	int ray_count = 0;

	try {
		for (int i = 0; i < depth; i++) {
			Ray next_ray = Ray(iter_src, iter_dir);
			stackTracker.addRay(next_ray);

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
				col = col * scene.environment->interact(next_ray, packet);

				PixelData result;
				result.boundHit = packet.aabb_check;
				result.shapeHit = packet.shape_check;
				result.color = col;

				return result;
			}
			else {
				if (!isDeterministic(packet.material)) {
					if (dot(next_ray.d, packet.normal) >= -epsilon) {
						packet.normal = packet.normal * -1.0f;
					}

					sample = RIS(scene, next_ray, packet);

					packet.material->interact(next_ray, sample, packet);
					ray_count++;

					if (dot(next_ray.d, packet.normal) >= epsilon) {
						std::stringstream log;
						log << "Normal: " << to_string(packet.normal) << "\n"
							<< "Next ray: " << to_string(packet.next_dir) << "\n"
							<< "Current ray: " << to_string(next_ray.d) << "\n"
							<< "Current pos: " << to_string(packet.last_hit) << "\n"
							<< "Last pos: " << to_string(next_ray.o) << "\n"
							<< "dot: " << dot(next_ray.d, packet.normal) << "\n"
							<< "Material name: " << packet.material->name << "\n\n";
						std::cout << log.str();
						stackTracker.LOG("Cos wo - normal", packet.normal.dot(sample.sample));
						throw stackTracker.throwException("Ray hit back face");
					}
				}
				else {
					sample.inv_pdf = 1.0f;
					packet.material->interact(next_ray, sample, packet);
					ray_count++;
					i--;
				}

				col = col * packet.color * sample.inv_pdf;

				if (debug) {
					std::cout << "Hit pos			: " << to_string(packet.last_hit) << "\n";
					std::cout << "Material hit		: " << packet.material->name << "\n";
					std::cout << "Incoming ray		: " << to_string(iter_dir) << "\n";
					std::cout << "Normal			: " << to_string(packet.normal) << "\n";
					std::cout << "Next ray			: " << to_string(packet.next_dir) << "\n";
					std::cout << "Dot normal		: " << packet.normal.dot(packet.next_dir) << "\n";
					std::cout << "Invert pdf		: " << sample.inv_pdf << "\n";
					std::cout << "Current ray color	: " << to_string(col) << "\n\n";
				}

				if (isnan(col.norm()) || col.x() < 0.0f || col.y() < 0.0f || col.z() < 0.0f) {
					stackTracker.LOG_STR("Color", to_string(packet.color));
					stackTracker.LOG("Cosine normal", packet.normal.dot(packet.next_dir));
					stackTracker.LOG("Invert PDF", sample.inv_pdf);

					throw stackTracker.throwException(std::string("Negative color value or NaN\n") + to_string(packet.color));
				}

				if (packet.normal.dot(packet.next_dir) > epsilon) iter_src = packet.last_hit + packet.normal * 0.001f;
				else if (packet.normal.dot(packet.next_dir) < -epsilon) iter_src = packet.last_hit + packet.normal * -0.001f;

				iter_dir = packet.next_dir;
			}
		}
	}
	catch (StackTrace* e) {
		PixelData result;
		result.boundHit = packet.aabb_check;
		result.shapeHit = packet.shape_check;
		result.color = { 10000000.0f, 0.0f, 0.0f, 1.0f };
		std::cout << e->toString() << "\n\n";
		return result;
	}

	PixelData result;
	result.boundHit = packet.aabb_check;
	result.shapeHit = packet.shape_check;
	result.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	return result;
}

PixelData trace_direct(const Scene& scene, const Ray& ray, const int& depth, bool debug) {
	vec3 iter_src = ray.o;
	vec3 iter_dir = ray.d;

	SampleResult sample;

	Packet packet{};
	RayStackTrace stackTrace;
	packet.debugger = (StackTrace*) &stackTrace;

	Ray next_ray = Ray(iter_src, iter_dir);
	if (!find_hit(scene, next_ray, packet)) {
		PixelData result;
		result.boundHit = packet.aabb_check;
		result.shapeHit = packet.shape_check;
		result.color = scene.environment->interact(next_ray, packet);
		return result;
	}
	else {
		if (!isDeterministic(packet.material)) {
			sample.sample = packet.normal;
			packet.material->interact(next_ray, sample, packet);
		}
		else {
			sample.inv_pdf = 1.0f;
			packet.material->interact(next_ray, sample, packet);
		}

		Color color = packet.color;
		color = color * (0.5f + 0.5f * packet.normal.dot(vec3(0.0f, 1.0f, 0.0f)));

		PixelData result;
		result.boundHit = packet.aabb_check;
		result.shapeHit = packet.shape_check;
		result.color = color;
		return result;
	}
}

static vec3 calculateNewRay(const Camera& cam, int current) {
	vec3 current_ray = cam.topLeft + (cam.down * random0to1() + cam.right * random0to1());
	current_ray = current_ray + cam.down * (cam.resolution.y - (current / (int) cam.resolution.x))
		+ cam.right * (current % (int) cam.resolution.x);
	current_ray.normalize();
	return current_ray;
}

PixelData shootRay(const Scene& object, int current, int depth, bool debug) {
	vec3 current_ray = calculateNewRay(object.camera, current);
	return trace(object, Ray(object.camera.pos, current_ray), depth, debug);
}

PixelData shootRayD(const Scene& object, int current, int depth, bool debug) {
	vec3 current_ray = calculateNewRay(object.camera, current);
	return trace_direct(object, Ray(object.camera.pos, current_ray), depth, debug);
}