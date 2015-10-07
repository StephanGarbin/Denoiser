#include <iostream>
#include <math.h>
#include <vector>

#include <fftw3.h>

#include "common.h"
#include "Transforms.h"

#define SQRT2     1.414213562373095
#define SQRT2_INV 0.7071067811865475

namespace Denoise
{

	void hardThreshold(float* block, float stdDeviation, index_t patchSize, index_t numPatches,
		float& weight);

	void ht_filtering_hadamard(
		std::vector<float> &group_3D
		, std::vector<float> &tmp
		, const unsigned nSx_r
		, const unsigned kHard
		, const unsigned chnls
		, std::vector<float> const& sigma_table
		, const float lambdaHard3D
		, std::vector<float> &weight_table
		, const bool doWeight
		);

	void normaliseDCT_NEW(float* block, index_t patchSize, index_t numPatches, bool inverse);

	void dctTransform(float* block, float* result, index_t patchSize, index_t numPatches);
	void normaliseDCT(float* block, bool inverse, index_t patchSize, index_t numPatches);
	void inverseDctTransform(float* block, float* result, index_t patchSize, index_t numPatches);

	void hadamard_transform(
		std::vector<float> &vec
		, std::vector<float> &tmp
		, const unsigned N
		, const unsigned D
		);

	void bm3dDEBUG(float* block, float stdDeviation, index_t patchSize, index_t numPatches,
		float& weight);
}
