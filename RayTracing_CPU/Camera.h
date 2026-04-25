#pragma once

#include "rtmath.h"

constexpr float DEGREE_TO_RADIAN = 3.14159265359 / 180.0f;

struct Camera {
	vec3 pos;
	vec3 dir;
	vec2 resolution;

	float fov;
	float aspect_ratio;
	float rot;

	vec3 topLeft;
	vec3 right;
	vec3 down;

	Camera() = default;

	Camera(vec3 position, vec3 direction, vec2 resolution, float fov) : pos(position), dir(direction), resolution(resolution), fov(fov) {
		this->resolution = resolution;
		aspect_ratio = resolution.x / (float)resolution.y;
		updateCamera(pos, dir);
	}

	void updateCamera(vec3 newPos, vec3 newDir) {
		dir = newDir;
		pos = newPos;

		vec3 left = vec3(dir.z(), 0.0f, -dir.x());
		left.normalize();
		left = left * tanf(fov * DEGREE_TO_RADIAN);
		
		vec3 top = dir.cross(left);
		left = left * aspect_ratio;

		topLeft = dir + top + left;
		right = left * (-2.0 / resolution.x);
		down = top * (-2.0 / resolution.y);
	}
};
