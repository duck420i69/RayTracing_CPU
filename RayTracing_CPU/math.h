#pragma once

#include <Eigen/Dense>
#include <random>
#include <cmath>

constexpr float epsilon = 0.00001f;
constexpr float pi = EIGEN_PI;
constexpr float infinity = std::numeric_limits<float>::infinity();

struct vec2 {
	float x, y;
	vec2 operator-(const vec2& other) const;
	vec2 operator+(const vec2& other) const;
	vec2 operator*(const float& other) const;
	void normalize();
};

using vec3 = Eigen::Vector3f;

struct vec4 {
	Eigen::Vector3f v;
	float w;

	inline vec4() {
		v << 0.0f, 0.0f, 0.0f;
		w = 1.0f;
	}

	inline vec4(Eigen::Vector3f vector3, float w) {
		v = vector3;
		this->w = 1.0f;
	}

	inline vec4(float x, float y, float z, float w) {
		v << x, y, z;
		this->w = w;
	}

	inline vec4 operator-(const vec4& other) const {
		return { v - other.v, 1.0f };
	}

	inline vec4 operator+(const vec4& other) const {
		return { v + other.v, 1.0f };
	}

	inline float norm() {
		return v.norm();
	}

	inline vec4 cross(const vec4& other) const {
		return { v.cross(other.v), 1.0f };
	}

	inline float dot(const vec4& other) const {
		return v.dot(other.v);
	}

	inline void normalize() {
		v.normalize();
	}
};

inline vec4 operator*(const vec4& vec, const float& other) {
	return { vec.v * other, vec.w };
}

inline vec4 operator*(const vec4& vec, const vec4& other) {
	return { vec.v[0] * other.v[0], vec.v[1] * other.v[1], vec.v[2] * other.v[2], vec.w };
}


inline vec4 reflect(const vec4& dir, const vec4& nor) {
	float temp = -2 * dir.dot(nor);
	return dir + nor * temp;
}

// refraction_ratio = in / out
inline vec4 refract(const vec4& dir, const vec4& nor, const float& refraction_ratio) {
	float cos_theta = dir.dot(nor * -1.0f);
	float sin_theta = sqrt(1 - cos_theta * cos_theta);
	if (sin_theta * refraction_ratio > 1.0f) return reflect(dir, nor);
	vec4 r_out_perp = (dir + nor * cos_theta) * refraction_ratio;
	vec4 r_out_parallel = nor * -std::sqrt(1.0f - r_out_perp.dot(r_out_perp));
	return r_out_perp + r_out_parallel;
}