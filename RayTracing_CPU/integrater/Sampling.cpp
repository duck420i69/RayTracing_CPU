#include "Sampling.h"


void Reservoir::addSample(const SampleResult& sample, float weight) {
	total_weight += weight;
	M++;
	if (random0to1() < weight / total_weight) {
		this->sample = sample;
		sample_weight = weight;
	}
}


void Reservoir::combineReservoir(const Reservoir& other) {
	total_weight += other.total_weight;
	M += other.M;
	if (random0to1() < other.total_weight / total_weight) {
		this->sample = other.sample;
		sample_weight = other.sample_weight;
	}
}
