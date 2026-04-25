#include "pch.h"
#include "CppUnitTest.h"

#include "integrater/Renderer.h"
#include "material/MaterialBuilder.h"
#include "chi_sqr.h"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


// Helper: fill approximate PDF per bin by uniform sampling inside each bin
static void computeApproxPdfBins(std::shared_ptr<Material> material, const Ray& ray, const Packet& packet,
    std::vector<std::vector<SampleBin>>& sampleBins, size_t h, size_t w, int approxSamplesPerBin)
{
    const float dy = pi / h / approxSamplesPerBin;
    const float dx = TwoPi / w / approxSamplesPerBin;

    for (size_t i = 0; i < h; ++i) {
        for (size_t j = 0; j < w; ++j) {
            sampleBins[i][j].sample_count = 0;
            sampleBins[i][j].approx_pdf = 0.0f;

            for (int sx = 0; sx < approxSamplesPerBin; ++sx) {
                for (int sy = 0; sy < approxSamplesPerBin; ++sy) {
                    vec2 sph = { sy * dy + i * pi / h, sx * dx + j * TwoPi / w };
                    sampleBins[i][j].approx_pdf += material->samplePDF(toCartesianCoordinate(sph), ray.d, packet.normal);
                }
            }

            sampleBins[i][j].approx_pdf /= (approxSamplesPerBin * approxSamplesPerBin);
        }
    }
}

// Helper: perform empirical sampling and accumulate counts + empirical pdf estimate per bin
static void sampleEmpirical(std::shared_ptr<Material> material, const Ray& ray, const Packet& packet,
    std::vector<std::vector<SampleBin>>& sampleBins, size_t h, size_t w, size_t empiricalSamples)
{
    for (size_t i = 0; i < empiricalSamples; ++i) {
        auto sample = material->generateSample(ray, packet);

        vec2 s = toSphericalCoordinate(sample.sample);
        size_t u = static_cast<size_t>(s.x / pi * h) % h;
        size_t v = static_cast<size_t>(s.y / TwoPi * w) % w;

        sampleBins[u][v].sample_count++;
        sampleBins[u][v].emp_pdf += 1.0f / sample.inv_pdf;
    }
}

// Helper: analyze bins and print statistics; may throw StatisticResult when discrepancy is large
static void analyzeBins(std::vector<std::vector<SampleBin>>& sampleBins, size_t h, size_t w, size_t empiricalSamples)
{
    const float dt = pi / h;
    const float dp = TwoPi / w;

    float chi = 0.0f;
    float chi_mean = 0.0f;
    float chi_var = 0.0f;
    float chi_min = 1000.0f;
    float chi_max = 0.0f;
    float brdf_integral = 0.0f;

    for (size_t i = 0; i < h; ++i) {
        for (size_t j = 0; j < w; ++j) {
            SampleBin& sample = sampleBins[i][j];

            try {
                StatisticResult stats{};

                float area = (cos(i * dt) - cos(i * dt + dt)) * dp;

                stats.approx_cdf = sample.approx_pdf * area;
                stats.expected_count = stats.approx_cdf * empiricalSamples;

                if (sample.sample_count > 3) {
                    sample.emp_pdf /= sample.sample_count;
                    stats.emp_cdf = sample.emp_pdf * area;
                }

                stats.sample = sample;

                if (stats.expected_count <= 0.0001f) {
                    stats.expected_count += 0.5f;
                }

                stats.chi_square = powf(sample.sample_count - stats.expected_count, 2) / (stats.expected_count);
                sample.chi_square = stats.chi_square;

                chi_mean += sample.chi_square;

                if (stats.chi_square > 10.0f) {
                    stats.reason = "chi square too high";
                }

                if (sample.sample_count > 3) {
                    if ((stats.approx_cdf - stats.emp_cdf) / stats.emp_cdf > 0.5f) {
                        stats.reason = "approx != empirical";
                        throw stats;
                    }
                }
            }
            catch (StatisticResult stats) {
                std::cout << "################" << std::endl;
                std::cout << "[" << i << ", " << j << "] " << stats.reason << std::endl << std::endl;
                std::cout << "[" << i << ", " << j << "] - approx PDF     : " << stats.sample.approx_pdf << std::endl;
                std::cout << "[" << i << ", " << j << "] - empirical PDF  : " << stats.sample.emp_pdf << std::endl;
                std::cout << "[" << i << ", " << j << "] + approx CDF     : " << stats.approx_cdf << std::endl;
                std::cout << "[" << i << ", " << j << "] + empirical CDF  : " << stats.emp_cdf << std::endl;
                std::cout << "[" << i << ", " << j << "] + similarity CDF : " << (stats.approx_cdf - stats.emp_cdf) / stats.emp_cdf << std::endl;
                std::cout << "[" << i << ", " << j << "] - chi square     : " << stats.chi_square << std::endl;
                std::cout << "[" << i << ", " << j << "] + count          : " << stats.sample.sample_count << std::endl;
                std::cout << "[" << i << ", " << j << "] + expected       : " << stats.expected_count << std::endl << std::endl << std::endl;
            }
        }
    }

    chi = chi_mean;
    chi_mean /= h * w;
    for (size_t i = 0; i < h; ++i) {
        for (size_t j = 0; j < w; ++j) {
            float chi_square = sampleBins[i][j].chi_square;

            if (chi_square > chi_max) {
                chi_max = chi_square;
            }
            if (chi_square < chi_min) {
                chi_min = chi_square;
            }
            chi_var += (chi_square - chi_mean) * (chi_square - chi_mean);

            const float area = (cos(i * dt) - cos(i * dt + dt)) * dp;
            brdf_integral += area * sampleBins[i][j].approx_pdf;
        }
    }

    chi_var /= h * w - 1;

    std::cout << "Chi value: " << chi << std::endl;
    std::cout << "BRDF integral: " << brdf_integral << std::endl;
    std::cout << "Mean: " << chi_mean << std::endl;
    std::cout << "Variance: " << chi_var << std::endl;
    std::cout << "Standard Variance: " << sqrtf(chi_var) << std::endl;
    std::cout << "Max: " << chi_max << std::endl;
    std::cout << "Min: " << chi_min << std::endl << std::endl;
}


void testMaterial(std::shared_ptr<Material>& material) {
    Packet packet;
    packet.material = material;
    packet.normal = { 0.0f, 1.0f, 0.0f };

    Ray ray = Ray({}, { 0.0f, -1.0f, 0.0f });

    const size_t h = 100;
    const size_t w = 100;
    auto sampleBins = std::vector<std::vector<SampleBin>>(h, std::vector<SampleBin>(w, SampleBin()));

    const int approxSamplesPerBin = 10; // used to compute approximate PDF inside each bin
    const size_t empiricalSamples = 100000; // reduced for test-run speed

    // build approx PDF map
    computeApproxPdfBins(material, ray, packet, sampleBins, h, w, approxSamplesPerBin);

    // empirical sampling
    sampleEmpirical(material, ray, packet, sampleBins, h, w, empiricalSamples);

    // analysis and reporting
    analyzeBins(sampleBins, h, w, empiricalSamples);
}



namespace IntegrateTest {
    TEST_CLASS(IntegrateTest) {
public:
    TEST_METHOD(ChiSqrMaterial) {
        auto runAndAssert = [&](std::shared_ptr<Material> mat) {
            try {
                testMaterial(mat);
            }
            catch (const StatisticResult& stats) {
                std::ostringstream oss;
                oss << "Material '" << mat->name << "' failed statistical test: " << stats.reason;
                std::string msg = oss.str();
                std::wstring wmsg(msg.begin(), msg.end());
                Assert::Fail(wmsg.c_str());
            }
            catch (const std::exception& ex) {
                std::ostringstream oss;
                oss << "Material '" << mat->name << "' threw exception: " << ex.what();
                std::string msg = oss.str();
                std::wstring wmsg(msg.begin(), msg.end());
                Assert::Fail(wmsg.c_str());
            }
        };

        MaterialBuilder builder;

        // Matte
        auto material = builder.setDiffuseColor({ 1.0f, 1.0f, 1.0f, 1.0f })
            .setSpecularColor({ 1.0f, 1.0f, 1.0f, 1.0f })
            .setRefraction(1.2f)
            .setShininess(100.0f)
            .setName("Matte")
            .build();

        runAndAssert(material);

        // GGX
        material = builder.setDiffuseColor({ 1.0f, 1.0f, 1.0f, 1.0f })
            .setSpecularColor({ 1.0f, 1.0f, 1.0f, 1.0f })
            .setRefraction(1.2f)
            .setShininess(100.0f)
            .setName("GGX")
            .build();

        runAndAssert(material);
    }
    };
}