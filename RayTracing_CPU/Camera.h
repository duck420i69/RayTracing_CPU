#pragma once

#include "Renderer.h"

constexpr float DEGREE_TO_RADIAN = 3.14159265359 / 180.0f;

struct Camera {
	vec4 pos;
	vec4 dir;
	vec2 resolution;

	float fov;
	float aspect_ratio;
	float rot;

	vec4 topleft;
	vec4 right;
	vec4 down;

	Camera() {}

	Camera(vec4 position, vec4 direction, vec2 resolution, float fov1) {
		pos = position;
		dir = direction;
		this->resolution = resolution;
		aspect_ratio = resolution.x / (float)resolution.y;
		fov = fov1;
		updateCamera(pos, dir);
	}

	void updateCamera(vec4 new_pos, vec4 new_dir) {
		vec4 left = { new_dir.v(2), 0.0f, -new_dir.v(0), 0.0f};
		left.normalize();
		left = left * tanf(fov * DEGREE_TO_RADIAN);
		vec4 top(0.0f, 0.0f, 0.0f, 1.0f);
		top = new_dir.cross(left);
		left = left * aspect_ratio;
		topleft = new_dir + top + left;
		right = left * (-2 / resolution.x);
		down = top * (-2 / resolution.y);

		dir = new_dir;
		pos = new_pos;
	}
};

PixelData shootRay(const Scene& object, const Camera& cam, int current, int depth, bool debug = false);

PixelData shootRayD(const Scene& object, const Camera& cam, int current, int depth, bool debug = false);