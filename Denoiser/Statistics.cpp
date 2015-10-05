#include "Statistics.h"

#include <cmath>
#include <iostream>

namespace Denoise
{
	float calculateBlockVariance(float* block, index_t numPatches, index_t patchSize)
	{
		//float std = 0.0f;
		//float mean = 0.0f;

		//for (index_t patch = 0; patch < numPatches; ++patch)
		//{
		//	for (index_t i = 0; i < std::pow(patchSize, 2); ++i)
		//	{
		//		std += std::pow(block[patch * sqr(patchSize) + i], 2);
		//		mean += block[patch * sqr(patchSize) + i];

		//	}
		//}
		//std::cout << "Std: " << std << "Mean: " << mean << "; ";
		//return (std - mean * mean / (float)(patchSize * numPatches)) / (float)(patchSize * numPatches - 1);

		float mean = 0.0f;

		for (index_t patch = 0; patch < numPatches; ++patch)
		{
			for (index_t i = 0; i < std::pow(patchSize, 2); ++i)
			{
				mean += block[patch * sqr(patchSize) + i];
			}
		}

		mean /= (float)(sqr(patchSize) * numPatches);

		float std = 0.0f;

		for (index_t patch = 0; patch < numPatches; ++patch)
		{
			for (index_t i = 0; i < std::pow(patchSize, 2); ++i)
			{
				std += std::pow(block[patch * sqr(patchSize) + i] - mean, 2);
			}
		}
		
		return std / (float)(sqr(patchSize) * numPatches);
	}

	void setBlockToAveragePatch(float* block, index_t numPatches, index_t patchSize)
	{
		float* averagePatch = new float[sqr(patchSize)];
		for (index_t i = 0; i < sqr(patchSize); ++i)
		{
			averagePatch[i] = 0.0f;
		}

		for (index_t patch = 0; patch < numPatches; ++patch)
		{
			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				averagePatch[i] += block[patch * sqr(patchSize) + i];
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
				block[patch * sqr(patchSize) + i] = averagePatch[i];
			}
		}

		delete[] averagePatch;
	}
}