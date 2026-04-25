#include "../rtmath.h"
#include "../Ray.h"
#include "../integrater/Sampling.h"

#include "GGX.h"
#include "Material.h"

#include <cmath>
#include <cstdlib>
#include <Eigen/src/Core/MatrixBase.h>


inline float GGX::lambdaGGX(const vec3& v, const vec3& h, const vec3& n, float alpha) const {
	float hdotv = dot(h, v);
	if (1.0f - hdotv < epsilon) return 1.0f;
	float a = (m_alpha * sqrt(1.0f - hdotv * hdotv)) / hdotv;
	return (sqrt(1.0f + a * a) - 1.0f) * 0.5f;
}


inline float GGX::ggxG1(const vec3& v, const vec3& h, const vec3& n, float alpha) const {
	float hdotv = dot(h, v);
	float a2 = m_alpha * m_alpha;
	return 2.0f * hdotv / (hdotv + sqrt(a2 + (1.0f - a2) * hdotv * hdotv));
}


inline float GGX::ggxD(const vec3& h, const vec3& n, float alpha) const {
	float ndoth = dot(n, h);
	float a2 = alpha * alpha;
	float t = ndoth * ndoth * (a2 - 1.0f) + 1.0f;

	return a2 / (pi * t * t);
}


vec4 GGX::evalBrdf(const vec3& wi, const vec3& wo, const vec3& normal) const {
	vec3 h = (wi + wo).normalized();

	if (dot(wi, normal) < epsilon || dot(wi, h) < epsilon)
		return { 0.0f, 0.0f, 0.0f, 1.0f };

	//float pdf = roughnessPdf(m_alpha, normal, h);
	float d = ggxD(h, normal, m_alpha);
	float g = ggxG1(wi, h, normal, m_alpha) * ggxG1(wo, h, normal, m_alpha);
	float f = Fresnal(refraction, 1.0f, dot(h, wo)); // 1.0f is the refractive index of air

	return (1.0f - f) * diffuse_color + specular_color * (d * g * f) / (4.0f * dot(wi, normal)) * dot(wo, normal);
}


SampleResult GGX::generateSample(const Ray& ray, const Packet& packet) const {
	float u = random0to1();
	float a2 = m_alpha * m_alpha;
	float cosTheta = sqrt((1.0f - u) / (u * (a2 - 1.0f) + 1.0f));
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
	float phi = random0to1() * TwoPi;
	vec3 random_direction = { sinTheta * std::cos(phi), cosTheta, sinTheta * std::sin(phi) };

    SampleResult result;

	// half-vector sampled in local space
	float y = ((a2 - 1.0f) * cosTheta * cosTheta + 1.0f);

	const vec3& normal = packet.normal;

	// transform half-vector to world space
	vec3 h = changeBase(normal, random_direction);

	// reflect the incoming direction around the half-vector to get the outgoing sample
	vec3 outgoing = reflect(ray.d, h);

	// compute pdf for the outgoing direction using the change of variables from h to outgoing
	float D = ggxD(h, normal, m_alpha);
	float cosThetaH = std::max(dot(h, normal), 0.0f);
	float absDot = std::abs(dot(outgoing, h));

	float pdf_out = 0.0f;
	if (absDot > 0.0f) {
		pdf_out = D * cosThetaH / (4.0f * absDot);
	}

	// store inverse pdf (used elsewhere as inv_pdf)
	result.inv_pdf = pdf_out > 0.0f ? 1.0f / pdf_out : std::numeric_limits<float>::infinity();

	result.sample = outgoing;

	return result;
}


float GGX::samplePDF(const vec3& wi, const vec3& wo, const vec3& normal) const {
    // For microfacet reflection, the half-vector is h = normalize(wi + wo)
	vec3 h = (wi + wo).normalized();

	float D = ggxD(h, normal, m_alpha);
	float cosThetaH = std::max(dot(h, normal), 0.0f);

	float absDot = std::abs(dot(wo, h));
	if (absDot <= 0.0f) return 0.0f;

	// pdf of sampling the outgoing direction when sampling the half-vector h
	float pdf = D * cosThetaH / (4.0f * absDot);
	return pdf;
}


void GGX::interact(const Ray& ray, const SampleResult& sample, Packet& packet) const {
	packet.next_dir = sample.sample;

	if (texture_map.size() > 0) {
		packet.color = getTexture({ packet.u, packet.v }) * evalBrdf(packet.next_dir, ray.d * (-1.0f), packet.normal) * packet.normal.dot(packet.next_dir);
	}
	else {
		packet.color = evalBrdf(packet.next_dir, ray.d * (-1.0f), packet.normal) * packet.normal.dot(packet.next_dir);
	}
}

