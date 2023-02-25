#include "Camera.h"
#include <iostream>


PixelData shootRay(const Scene& object, const Camera& cam, int current, int depth, bool debug) {
	vec4 current_ray = cam.topleft + (cam.down * random0to1() + cam.right * random0to1());
	current_ray = current_ray + cam.down * (cam.resolution.y - (current / (int)cam.resolution.x)) + cam.right * (current % (int)cam.resolution.x);
	current_ray.normalize();
	return trace(object, Ray(cam.pos, current_ray), depth, debug);
}

PixelData shootRayD(const Scene& object, const Camera& cam, int current, int depth, bool debug) {
	vec4 current_ray = cam.topleft + (cam.down * random0to1() + cam.right * random0to1());
	current_ray = current_ray + cam.down * (cam.resolution.y - (current / (int)cam.resolution.x)) + cam.right * (current % (int)cam.resolution.x);
	current_ray.normalize();
	return trace_direct(object, Ray(cam.pos, current_ray), depth, debug);
}