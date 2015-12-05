#pragma once

#include "common.h"
#include "TRANSFORM_DOMAIN_FORMATS.h"


namespace Denoise
{
	DOMAIN_FORMAT calculateBlockVariance(DOMAIN_FORMAT* block, index_t numPatches, index_t patchSize, index_t numChannels);

	void setBlockToAveragePatch(DOMAIN_FORMAT* block, index_t numPatches, index_t patchSize, index_t numChannels);
	void setBlockToAverage(DOMAIN_FORMAT* block, index_t numPatches, index_t patchSize, index_t numChannels);

	void calculateBlockMeans(DOMAIN_FORMAT* block, index_t numPatches, index_t patchSize, index_t numChannels,
		DOMAIN_FORMAT* means);

	DOMAIN_FORMAT calculateBlockMean(DOMAIN_FORMAT* block, index_t numPatches, index_t patchSize, index_t numChannels);

	DOMAIN_FORMAT calculateMeanAdaptiveFactor(DOMAIN_FORMAT stdDeviaton, DOMAIN_FORMAT mean, DOMAIN_FORMAT scaling, DOMAIN_FORMAT power);
}

