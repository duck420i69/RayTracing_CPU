#pragma once

#include "../Scene.h"
#include "../Buffer.h"
#include "../Camera.h"

#include "Sampling.h"

bool find_hit(const Scene& objects, const Ray& ray, Packet& result);

SampleResult RIS(const Scene& scene, const Ray& ray, Packet& packet);

PixelData trace(const Scene& scene, const Ray& ray, const int& depth, bool debug = false);

PixelData trace_direct(const Scene& scene, const Ray& ray, const int& depth, bool debug = false);

PixelData shootRay(const Scene& object, int current, int depth, bool debug = false);

PixelData shootRayD(const Scene& object, int current, int depth, bool debug = false);