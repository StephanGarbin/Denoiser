#include "Statistics.h"

#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

namespace Denoise
{
	DOMAIN_FORMAT calculateBlockVariance(DOMAIN_FORMAT* block, index_t numPatches, index_t patchSize, index_t numChannels)
	{
		index_t totalSize = sqr(patchSize) * numPatches;

		std::vector<DOMAIN_FORMAT> variances(numChannels);

		for (index_t c = 0; c < numChannels; ++c)
		{
			index_t colourOffset = c * totalSize;

			DOMAIN_FORMAT mean = 0.0f;

			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t i = 0; i < sqr(patchSize); ++i)
				{
					mean += block[colourOffset + patch * sqr(patchSize) + i];
				}
			}

			mean /= (DOMAIN_FORMAT)(sqr(patchSize) * numPatches);


			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t i = 0; i < sqr(patchSize); ++i)
				{
					variances[c] += std::pow(block[colourOffset + patch * sqr(patchSize) + i] - mean, 2);
				}
			}

			variances[c] /= (DOMAIN_FORMAT)totalSize;
		}
		
		return std::accumulate(variances.begin(), variances.end(), 0.0f) / (DOMAIN_FORMAT)variances.size();
	}

	void setBlockToAveragePatch(DOMAIN_FORMAT* block, index_t numPatches, index_t patchSize, index_t numChannels)
	{
		index_t totalSize = sqr(patchSize) * numPatches;
		DOMAIN_FORMAT* averagePatch = new DOMAIN_FORMAT[sqr(patchSize)];

		for (index_t c = 0; c < numChannels; ++c)
		{
			index_t colourOffset = c * totalSize;

			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				averagePatch[i] = 0.0f;
			}

			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t i = 0; i < sqr(patchSize); ++i)
				{
					averagePatch[i] += block[colourOffset + patch * sqr(patchSize) + i];
				}
			}

			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				averagePatch[i] /= (DOMAIN_FORMAT)numPatches;
			}

			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t i = 0; i < sqr(patchSize); ++i)
				{
					block[colourOffset + patch * sqr(patchSize) + i] = averagePatch[i];
				}
			}

		}
		delete[] averagePatch;
	}

	void calculateBlockMeans(DOMAIN_FORMAT* block, index_t numPatches, index_t patchSize, index_t numChannels,
		DOMAIN_FORMAT* means)
	{
		index_t totalSize = sqr(patchSize) * numPatches;
		DOMAIN_FORMAT* averagePatch = new DOMAIN_FORMAT[sqr(patchSize)];

		for (index_t c = 0; c < numChannels; ++c)
		{
			index_t colourOffset = c * totalSize;

			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				averagePatch[i] = 0.0f;
			}

			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t i = 0; i < sqr(patchSize); ++i)
				{
					averagePatch[i] += block[colourOffset + patch * sqr(patchSize) + i];
				}
			}

			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				averagePatch[i] /= (DOMAIN_FORMAT)numPatches;
			}


			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				means[c * sqr(patchSize) + i] = averagePatch[i];
			}

		}
		delete[] averagePatch;
	}

	DOMAIN_FORMAT calculateBlockMean(DOMAIN_FORMAT* block, index_t numPatches, index_t patchSize, index_t numChannels)
	{
		DOMAIN_FORMAT mean = 0.0f;
		index_t totalSize = sqr(patchSize) * numPatches;
		DOMAIN_FORMAT* averagePatch = new DOMAIN_FORMAT[sqr(patchSize)];

		for (index_t c = 0; c < numChannels; ++c)
		{
			index_t colourOffset = c * totalSize;

			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				averagePatch[i] = 0.0f;
			}

			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t i = 0; i < sqr(patchSize); ++i)
				{
					averagePatch[i] += block[colourOffset + patch * sqr(patchSize) + i];
				}
			}

			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				averagePatch[i] /= (DOMAIN_FORMAT)numPatches;
			}


			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				mean += averagePatch[i];
			}

		}
		delete[] averagePatch;

		return mean / (numChannels * sqr(patchSize));
	}

	DOMAIN_FORMAT calculateMeanAdaptiveFactor(DOMAIN_FORMAT stdDeviaton, DOMAIN_FORMAT mean, DOMAIN_FORMAT scaling, DOMAIN_FORMAT power)
	{
		return 1.0f + std::pow(mean, power) * scaling;
		//return 1.0f / 3.0f + (mean / 3.0f) * scaling;
	}
}