#pragma once

#include "pch.h"


inline vec2 toSphericalCoordinate(vec3 v) {
    float phi = atan2f(-v.z(), -v.x()) + pi;
    float theta = abs(v.y()) >= 1.0f ? PiOver2 - PiOver2 * v.y() : acosf(v.y());

    return { theta, phi };
}

inline vec3 toCartesianCoordinate(vec2 v) {
    if (v.x >= pi) v.x -= pi;
    if (v.x >= TwoPi) v.x -= TwoPi;

    return { sin(v.x) * cos(v.y), cos(v.x), sin(v.x) * sin(v.y) };
}


struct SampleBin {
    float approx_pdf;
    float emp_pdf;
    size_t sample_count;

    float chi_square;
};


struct StatisticResult {
    SampleBin sample;

    float expected_count;
    float chi_square;

    float approx_cdf;
    float emp_cdf;

    std::string reason;
};


//void chiSqrTest2D(int x, int y, int ()) {
//    Packet packet;
//    packet.material = material;
//    packet.normal = { 0.0f, 1.0f, 0.0f };
//
//    Ray ray = Ray({}, { 0.0f, -1.0f, 0.0f });
//
//    size_t h = 100;
//    size_t w = 100;
//    auto sampleBins = std::vector<std::vector<SampleBin>>(100, std::vector<SampleBin>(100, SampleBin()));
//
//    const float dt = pi / h;
//    const float dp = TwoPi / w;
//
//
//    for (size_t i = 0; i < 100; i++) {
//        for (size_t j = 0; j < 100; j++) {
//            const int sample_size = 10;
//            const float dy = pi / h / sample_size;
//            const float dx = TwoPi / w / sample_size;
//
//            sampleBins[i][j].sample_count = 0;
//            sampleBins[i][j].approx_pdf = 0;
//
//            for (size_t x = 0; x < sample_size; x++) {
//                for (size_t y = 0; y < sample_size; y++) {
//                    sampleBins[i][j].approx_pdf += material->samplePDF(
//                        toCartesianCoordinate({ y * dy + i * pi / h, x * dx + j * TwoPi / w }), ray.d, packet.normal);
//                }
//            }
//
//            sampleBins[i][j].approx_pdf /= sample_size * sample_size;
//        }
//    }
//
//    const size_t sample_size = 10000000;
//
//    for (size_t i = 0; i < sample_size; i++) {
//        auto sample = material->generateSample(ray, packet);
//
//        vec2 s = toSphericalCoordinate(sample.sample);
//        size_t u = static_cast<size_t>(s.x / pi * h) % h;
//        size_t v = static_cast<size_t>(s.y / TwoPi * w) % w;
//
//        sampleBins[u][v].sample_count++;
//        sampleBins[u][v].emp_pdf += 1.0f / sample.inv_pdf;
//    }
//
//    float chi = 0.0f;
//    float chi_mean = 0.0f;
//    float chi_var = 0.0f;
//    float chi_min = 1000.0f;
//    float chi_max = 0.0f;
//    float brdf_integral = 0.0f;
//
//    for (size_t i = 0; i < 100; i++) {
//        for (size_t j = 0; j < 100; j++) {
//            SampleBin& sample = sampleBins[i][j];
//
//            try {
//                StatisticResult stats{};
//
//                float area = (cos(i * dt) - cos(i * dt + dt)) * dp;
//
//                stats.approx_cdf = sample.approx_pdf * area;
//                stats.expected_count = stats.approx_cdf * sample_size;
//
//                if (sample.sample_count > 3) {
//                    sample.emp_pdf /= sample.sample_count;
//                    stats.emp_cdf = sample.emp_pdf * area;
//                }
//
//                stats.sample = sample;
//
//                if (stats.expected_count <= 0.0001f) {
//                    stats.expected_count += 0.5f;
//                }
//
//                stats.chi_square = powf(sample.sample_count - stats.expected_count, 2) / (stats.expected_count);
//                sample.chi_square = stats.chi_square;
//
//                chi_mean += sample.chi_square;
//
//                if (stats.chi_square > 10.0f) {
//                    stats.reason = "chi square too high";
//                    // throw stats;
//                }
//
//                if (sample.sample_count > 3) {
//                    if ((stats.approx_cdf - stats.emp_cdf) / stats.emp_cdf > 0.5f) {
//                        stats.reason = "approx != empirical";
//                        throw stats;
//                    }
//                }
//            }
//            catch (StatisticResult stats) {
//                std::cout << "################" << std::endl;
//                std::cout << "[" << i << ", " << j << "] " << stats.reason << std::endl << std::endl;
//                std::cout << "[" << i << ", " << j << "] - approx PDF     : " << stats.sample.approx_pdf << std::endl;
//                std::cout << "[" << i << ", " << j << "] - empirical PDF  : " << stats.sample.emp_pdf << std::endl;
//                std::cout << "[" << i << ", " << j << "] + approx CDF     : " << stats.approx_cdf << std::endl;
//                std::cout << "[" << i << ", " << j << "] + empirical CDF  : " << stats.emp_cdf << std::endl;
//                std::cout << "[" << i << ", " << j << "] + similarity CDF : " << (stats.approx_cdf - stats.emp_cdf) / stats.emp_cdf << std::endl;
//                std::cout << "[" << i << ", " << j << "] - chi square     : " << stats.chi_square << std::endl;
//                std::cout << "[" << i << ", " << j << "] + count          : " << stats.sample.sample_count << std::endl;
//                std::cout << "[" << i << ", " << j << "] + expected       : " << stats.expected_count << std::endl << std::endl << std::endl;
//            }
//        }
//    }
//
//    chi = chi_mean;
//    chi_mean /= h * w;
//    for (size_t i = 0; i < 100; i++) {
//        for (size_t j = 0; j < 100; j++) {
//            float chi_square = sampleBins[i][j].chi_square;
//
//            if (chi_square > chi_max) {
//                chi_max = chi_square;
//            }
//            if (chi_square < chi_min) {
//                chi_min = chi_square;
//            }
//            chi_var += (chi_square - chi_mean) * (chi_square - chi_mean);
//
//            const float area = (cos(i * dt) - cos(i * dt + dt)) * dp;
//            brdf_integral += area * sampleBins[i][j].approx_pdf;
//        }
//    }
//
//    chi_var /= h * w - 1;
//
//    std::cout << "Chi value: " << chi << std::endl;
//    std::cout << "BRDF integral: " << brdf_integral << std::endl;
//    std::cout << "Mean: " << chi_mean << std::endl;
//    std::cout << "Variance: " << chi_var << std::endl;
//    std::cout << "Standard Variance: " << sqrtf(chi_var) << std::endl;
//    std::cout << "Max: " << chi_max << std::endl;
//    std::cout << "Min: " << chi_min << std::endl << std::endl;
//}
