#pragma once

#include <vector>

#include "common.h"

namespace Denoise
{
	struct NLMeansSettings
	{
		std::vector<float> stdDeviation;

		index_t stepSizeCols;
		index_t stepSizeRows;

		index_t searchWindowSize;

		index_t MaxNumPatches;

		//Block Matching
		float templateMatchingMaxAllowedPatchDistance;
		int templateMatchingNorm;
		index_t templateMatchingNumChannels;

		index_t patchSize;


		//Multi-Threading
		index_t numThreadsBlockMatching;
		index_t numThreadsDenoising;

		void init2defaults(std::vector<float> smoothness, bool preview = false)
		{
			stdDeviation = smoothness;

			if (preview)
			{
				stepSizeCols = 8;
				stepSizeRows = 8;
				searchWindowSize = 16;
				MaxNumPatches = 16;
			}
			else
			{
				stepSizeCols = 3;
				stepSizeRows = 3;
				searchWindowSize = 32;
				MaxNumPatches = 32;
			}

			//Block Matching
			templateMatchingMaxAllowedPatchDistance = 2.5f;
			templateMatchingNorm = 2;
			templateMatchingNumChannels = 1;

			patchSize = 8;


			//Multi-Threading
			numThreadsBlockMatching = 1;
			numThreadsDenoising = 1;
		}

		void limitHardwareConcurrency(size_t numThreads)
		{
			numThreadsBlockMatching = numThreads;
			numThreadsDenoising = numThreads;
		}

		void previewQuality()
		{
			stepSizeCols = 8;
			stepSizeRows = 8;
			searchWindowSize = 16;
			MaxNumPatches = 16;
		}

		void productionQuality()
		{
			stepSizeCols = 3;
			stepSizeRows = 3;
			searchWindowSize = 32;
			MaxNumPatches = 32;
		}
	};
}