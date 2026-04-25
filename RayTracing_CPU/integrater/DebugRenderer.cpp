#include "../config.h"

#include "Renderer.h"
#include "DebugRenderer.h"

#include <iostream>


vec3 toCameraBase(const Camera& camera, const vec3& point) {
	const vec2& resolution = camera.resolution;
	vec3 v = point - camera.pos;
	float z = v.dot(camera.dir);
	
	vec3 result = {
		v.dot(camera.right) / (camera.right.norm() * camera.right.norm()),
		v.dot(camera.down)  / (camera.down.norm()  * camera.down.norm()),
		z
	};
	return result;
}

inline vec3 toScreenSpace(const Camera& camera, const vec3& point) {
	return {
	point.x() / point.z() + 0.5f * camera.resolution.x,
	point.y() / point.z() + 0.5f * camera.resolution.y,
	point.z()
	};
}

void drawPixel(Buffer& buffer, const vec2& resolution, const vec3& p, const Color& color) {
	if (0 <= (int)p.x() && (int)p.x() < resolution.x && 0 <= (int)p.y() && (int)p.y() < resolution.y && p.z() > 0) {
		buffer.frameBuffer[(int)p.x() + (int)p.y() * buffer.w] = color;
	}
}

void drawLine(Buffer& buffer, const Camera& camera, const vec3& p1, const vec3& p2, const Color& color) {
	const vec2& resolution = camera.resolution;
	vec3 v1 = toCameraBase(camera, p1);
    vec3 v2 = toCameraBase(camera, p2);

	//std::cout << to_string(camera.pos) << std::endl;
	//std::cout << to_string(camera.dir) << std::endl;
	//std::cout << to_string(camera.right) << std::endl;
	//std::cout << to_string(camera.down) << std::endl;
	// 
	//std::cout << to_string(p1) << std::endl;
	//std::cout << to_string(v1) << std::endl;
	//std::cout << to_string(p2) << std::endl;
	//std::cout << to_string(v2) << std::endl;

	if (v1.z() <= 0.1f && v2.z() <= 0.1f) {
		return;
	}

	if (v1.z() < 0.1f) {
		float t = (0.1f - v1.z()) / (v2 - v1).z();
		v1 = v1 + (v2 - v1) * t;
	}

	if (v2.z() < 0.1f) {
		float t = (0.1f - v2.z()) / (v1 - v2).z();
		v2 = v2 + (v1 - v2) * t;
	}

	//vec4 dv = v2 - v1;
	//float t1 = (0.5f * resolution.x - v1.x()) / dv.x();
	//float t2 = (-0.5f * resolution.x - v1.x()) / dv.x();
	//float t3 = (0.5f * resolution.y - v1.y()) / dv.y();
	//float t4 = (-0.5f * resolution.y - v1.y()) / dv.y();

	//std::vector<float> t = { 0.0f, 1.0f, t1, t2, t3, t4 };
	//std::sort(t.begin(), t.end());

	//v1 = v1 + dv * t[2];
	//v2 = v1 + dv * t[3];

	v1 = toScreenSpace(camera, v1);
	v2 = toScreenSpace(camera, v2);

	if (v1.z() == 0.0f) {
		v1 = { 0.0f, 0.0f, 0.0f };
	}

	int x0 = v1.x(), x1 = v2.x();
	int y0 = v1.y(), y1 = v2.y();
	int dx = std::abs(x1 - x0);
	int sx = x0 < x1 ? 1 : -1;
	int dy = -std::abs(y1 - y0);
	int sy = y0 < y1 ? 1 : -1;
	int error = dx + dy;

	while (true) {
		if (0 <= x0 && x0 < resolution.x && 0 <= y0 && y0 < resolution.y) {
			buffer.frameBuffer[x0 + (resolution.y - y0 - 1) * buffer.w] = color;
		}
		if (x0 == x1 && y0 == y1) break;
		float e2 = 2 * error;
		if (e2 >= dy) {
			if (x0 == x1) break;
			error = error + dy;
			x0 = x0 + sx;
		}

		if (e2 <= dx) {
			if (y0 == y1) break;
			error = error + dx;
			y0 = y0 + sy;
		}
	}
}

void drawPoint(Buffer& buffer, const Camera& camera, const vec3& p, const Color& color) {
	const vec2& resolution = camera.resolution;
	vec3 v = toCameraBase(camera, p);
	v = toScreenSpace(camera, v);
	if (0 <= (int)v.x() && (int)v.x() < resolution.x && 0 <= (int)v.y() && (int)v.y() < resolution.y && v.z() > 0) {
		buffer.frameBuffer[(int)v.x() + (resolution.y - 1 - (int)v.y()) * buffer.w] = color;
	}
}

std::vector<Ray> traceDebug(const Scene& scene, const Ray& ray, const int& depth) {
	Color col = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec3 iter_src = ray.o;
	vec3 iter_dir = ray.d;

	RayStackTrace stackTracker;
	Packet packet{};
	packet.debugger = (StackTrace*)&stackTracker;

	SampleResult sample;
	PixelData result;

	int ray_count = 0;

	std::cout << "-------------------" << std::endl;
	for (int i = 0; i < depth; i++) {
		Ray next_ray = Ray(iter_src, iter_dir);
		stackTracker.addRay(next_ray);

		if (ray_count >= 20) {
			result.boundHit = packet.aabb_check;
			result.shapeHit = packet.shape_check;
			result.color = { 0.0f, 0.0f, 0.0f, 1.0f };
			std::cout << "-------------------" << std::endl;
			return stackTracker.rayPath;
		}

		std::cout << "Bounce: " << i + 1 << "\n\n";

		if (!find_hit(scene, next_ray, packet)) {
			std::cout << "Hit the sky! " << "\n\n";
			col = col * scene.environment->interact(next_ray, packet);

			result.boundHit = packet.aabb_check;
			result.shapeHit = packet.shape_check;
			result.color = col;
			std::cout << "-------------------" << std::endl;
			return stackTracker.rayPath;
		}
		else {
			if (!isDeterministic(packet.material)) {
				if (dot(next_ray.d, packet.normal) >= -epsilon) {
					packet.normal = packet.normal * -1.0f;
				}

				sample = RIS(scene, next_ray, packet);
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

			std::cout << "Hit pos			: " << to_string(packet.last_hit) << "\n";
			std::cout << "Material hitted	: " << packet.material->name << "\n";
			std::cout << "Incoming ray		: " << to_string(iter_dir) << "\n";
			std::cout << "Normal			: " << to_string(packet.normal) << "\n";
			std::cout << "Next ray			: " << to_string(packet.next_dir) << "\n";
			std::cout << "Dot normal		: " << packet.normal.dot(packet.next_dir) << "\n";
			std::cout << "Invert pdf		: " << sample.inv_pdf << "\n";
			std::cout << "Current ray color	: " << to_string(col) << "\n\n";


			if (packet.normal.dot(packet.next_dir) > epsilon) {
				iter_src = packet.last_hit + packet.normal * 0.001f;
			}
			else if (packet.normal.dot(packet.next_dir) < -epsilon) {
				iter_src = packet.last_hit + packet.normal * -0.001f;
			}

			iter_dir = packet.next_dir;
		}
	}

	result.boundHit = packet.aabb_check;
	result.shapeHit = packet.shape_check;
	result.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	std::cout << "-------------------" << std::endl;
	return stackTracker.rayPath;
}

std::vector<Ray> shootRayDebug(const Scene& object, int current, int depth) {
	const Camera& cam = object.camera;
	vec3 current_ray = cam.topLeft + (cam.down * random0to1() + cam.right * random0to1());
	current_ray = current_ray + cam.down * (cam.resolution.y - (current / (int)cam.resolution.x)) + cam.right * (current % (int)cam.resolution.x);
	current_ray.normalize();
	return traceDebug(object, Ray(cam.pos, current_ray), depth);
}