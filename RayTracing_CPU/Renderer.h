#pragma once

#include "Shape.h"
#include "Sampling.h"
#include "Buffer.h"

bool find_hit(const Scene& objects, const Ray& ray, Packet& result);

PixelData trace(const Scene& objects, const Ray& ray, const int& depth, bool debug);

PixelData trace_direct(const Scene& scene, const Ray& ray, const int& depth, bool debug);