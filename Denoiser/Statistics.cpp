#include "Statistics.h"

#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

namespace Denoise
{
	float calculateBlockVariance(float* block, index_t numPatches, index_t patchSize, index_t numChannels)
	{
		index_t totalSize = sqr(patchSize) * numPatches;

		std::vector<float> variances(numChannels);

		for (index_t c = 0; c < numChannels; ++c)
		{
			index_t colourOffset = c * totalSize;

			float mean = 0.0f;

			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t i = 0; i < std::pow(patchSize, 2); ++i)
				{
					mean += block[colourOffset + patch * sqr(patchSize) + i];
				}
			}

			mean /= (float)(sqr(patchSize) * numPatches);


			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t i = 0; i < std::pow(patchSize, 2); ++i)
				{
					variances[c] += std::pow(block[colourOffset + patch * sqr(patchSize) + i] - mean, 2);
				}
			}

			variances[c] /= (float)totalSize;
		}
		
		return std::accumulate(variances.begin(), variances.end(), 0.0f) / (float)variances.size();
	}

	void setBlockToAveragePatch(float* block, index_t numPatches, index_t patchSize, index_t numChannels)
	{
		index_t totalSize = sqr(patchSize) * numPatches;
		float* averagePatch = new float[sqr(patchSize)];

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
				averagePatch[i] /= (float)numPatches;
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

	void calculateBlockMeans(float* block, index_t numPatches, index_t patchSize, index_t numChannels,
		float* means)
	{
		index_t totalSize = sqr(patchSize) * numPatches;
		float* averagePatch = new float[sqr(patchSize)];

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
				averagePatch[i] /= (float)numPatches;
			}


			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				means[c * sqr(patchSize) + i] = averagePatch[i];
			}

		}
		delete[] averagePatch;
	}

	float calculateBlockMean(float* block, index_t numPatches, index_t patchSize, index_t numChannels)
	{
		float mean = 0.0f;
		index_t totalSize = sqr(patchSize) * numPatches;
		float* averagePatch = new float[sqr(patchSize)];

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
				averagePatch[i] /= (float)numPatches;
			}


			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				mean += averagePatch[i];
			}

		}
		delete[] averagePatch;

		return mean / (numChannels * sqr(patchSize));
	}

	float calculateMeanAdaptiveFactor(float stdDeviaton, float mean, float scaling)
	{
		return 1.0f + std::pow(mean, 3) * scaling;
		//return 1.0f / 3.0f + (mean / 3.0f) * scaling;
	}
}