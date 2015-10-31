#pragma once

#include "common.h"

namespace Denoise
{
	float calculateBlockVariance(float* block, index_t numPatches, index_t patchSize, index_t numChannels);
	void setBlockToAveragePatch(float* block, index_t numPatches, index_t patchSize, index_t numChannels);

	void calculateBlockMeans(float* block, index_t numPatches, index_t patchSize, index_t numChannels,
		float* means);

	float calculateMeanAdaptiveFactor(float stdDeviaton, float mean, float scaling);
}

