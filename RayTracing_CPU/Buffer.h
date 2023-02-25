#pragma once
#include "math.h"


class PixelData {
public:
	vec4 color;
	uint16_t boundHit;
	uint16_t shapeHit;
};

class Buffer {
public:
	Buffer(uint16_t width, uint16_t height) : w(width), h(height) {
		frameBuffer.resize(static_cast<size_t>(w) * h);
		boundHitBuffer.resize(static_cast<size_t>(w) * h);
		shapeHitBuffer.resize(static_cast<size_t>(w) * h);
	}

	uint16_t w, h;
	std::vector<vec4> frameBuffer;
	std::vector<uint16_t> boundHitBuffer;
	std::vector<uint16_t> shapeHitBuffer;
};