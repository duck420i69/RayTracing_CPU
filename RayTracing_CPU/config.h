#pragma once

#include <cstdint>


// Threading
constexpr bool SINGLE_THREAD = true;
constexpr int PIXEL_PER_THREAD = 20;

// Rendering
constexpr int DEPTH = 4;
constexpr float NEXT_RAY_SAMPLING_PROPABILITY = 0.9f;
constexpr uint8_t ENVIRONMENT_SAMPLE = 2;
constexpr uint8_t LIGHT_SAMPLE = 2;
constexpr uint8_t BRDF_SAMPLE = 2;