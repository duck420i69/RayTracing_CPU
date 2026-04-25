#pragma once

#include <Eigen/Dense>
#include <random>
#include <cmath>

constexpr float epsilon = 0.00002f;
constexpr float pi = EIGEN_PI;
constexpr float infinity = std::numeric_limits<float>::infinity();

struct vec2 {
	float x, y;
	vec2 operator-(const vec2& other) const;
	vec2 operator+(const vec2& other) const;
	vec2 operator*(const float& other) const;
	void normalize();
};

typedef Eigen::Vector3f vec3;

struct vec4 {
	Eigen::Vector3f v;
	float w;

	vec4() {
		v << 0.0f, 0.0f, 0.0f;
		w = 1.0f;
	}

	vec4(const vec4& other) {
		v = other.v;
		w = 1.0f;
	}

	vec4(Eigen::Vector3f vector3, float w) {
		v = vector3;
		this->w = 1.0f;
	}

	vec4(float x, float y, float z, float w) {
		v << x, y, z;
		this->w = w;
	}

	inline vec4& operator=(const vec4& other) {
		v = other.v;
		return *this;
	}

	inline vec4 operator-(const vec4& other) const {
		return { v - other.v, 1.0f };
	}

	inline vec4 operator+(const vec4& other) const {
		return { v + other.v, 1.0f };
	}

	inline float norm() const {
		return v.norm();
	}

	inline vec4 cross(const vec4& other) const {
		return { v.cross(other.v), 1.0f };
	}

	inline float dot(const vec4& other) const {
		return v.dot(other.v);
	}

	inline vec4& normalize() {
		v.normalize();
		return *this;
	}

	inline float& x() {
		return v.x();
	}

	inline float& y() {
		return v.y();
	}

	inline float& z() {
		return v.z();
	}

	inline float& r() {
		return v.x();
	}

	inline float& g() {
		return v.y();
	}

	inline float& b() {
		return v.z();
	}
};

inline vec4 operator*(const vec4& vec, const float& f) {
	return { vec.v * f, vec.w };
}

inline vec4 operator*(const float& f, const vec4& vec) {
	return { vec.v * f, vec.w };
}

inline vec4 operator/(const vec4& vec, const float& f) {
	float inv = 1.0f / f;
	return { vec.v / f, vec.w };
}

inline vec4 operator*(const vec4& vec, const vec4& other) {
	return { vec.v[0] * other.v[0], vec.v[1] * other.v[1], vec.v[2] * other.v[2], vec.w };
}

inline vec3 cross(const vec3& v1, const vec3& v2) {
	return v1.cross(v2);
}

inline float dot(const vec3& v1, const vec3& v2) {
	return v1.dot(v2);
}

inline vec3 reflect(const vec3& dir, const vec3& nor) {
	float temp = -2.0f * dir.dot(nor);
	return dir + nor * temp;
}

// refraction_ratio = in / out
inline vec3 refract(const vec3& dir, const vec3& nor, const float& refraction_ratio) {
	float cos_theta = dir.dot(nor * -1.0f);
	float sin_theta = sqrt(1 - cos_theta * cos_theta);
	if (sin_theta * refraction_ratio > 1.0f) return reflect(dir, nor);
	vec3 r_out_perp = (dir + nor * cos_theta) * refraction_ratio;
	vec3 r_out_parallel = nor * -std::sqrt(1.0f - r_out_perp.dot(r_out_perp));
	return r_out_perp + r_out_parallel;
}

inline vec3 changeBase(const vec3& upDirection, const vec3& v) {
	if (upDirection.y() == 1.0f) {
		return v;
	}

	if (upDirection.y() == -1.0f) {
		return vec3(v.x(), -v.y(), v.z());
	}

	vec3 x = upDirection.cross(vec3(0.0f, 1.0f, 0.0f)).normalized();
	vec3 z = upDirection.cross(x).normalized();

	return x * v.x() + upDirection * v.y() + z * v.z();
}

std::string to_string(vec3 v);

std::string to_string(vec4 v);


using texel = vec2;
using Color = vec4;