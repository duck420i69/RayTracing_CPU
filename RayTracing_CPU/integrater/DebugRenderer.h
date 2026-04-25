#pragma once

#include "../Camera.h"
#include "../Buffer.h"
#include "../Scene.h"

void drawLine(Buffer& buffer, const Camera& camera, const vec3& p1, const vec3& p2, const Color& color);

void drawPoint(Buffer& buffer, const Camera& camera, const vec3& p, const Color& color);

std::vector<Ray> traceDebug(const Scene& scene, const Ray& ray, const int& depth);

std::vector<Ray> shootRayDebug(const Scene& object, int current, int depth);