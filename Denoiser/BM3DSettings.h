#pragma once

#include <vector>

#include "common.h"

namespace Denoise
{
	struct BM3DSettings
	{
		bool disableWienerFilter;

		std::vector<float> stdDeviation;

		index_t stepSizeCols;
		index_t stepSizeRows;

		index_t searchWindowSize;

		index_t numPatchesPerBlockCollaborative;
		index_t numPatchesPerBlockWiener;

		//Block Matching
		float templateMatchingMaxAllowedPatchDistance;
		int templateMatchingNorm;
		index_t templateMatchingNumChannels;

		index_t patchSize;

		bool usePatchWeighting;

		//Statistical Extensions
		bool averageBlocksBasedOnStdCollaborative;
		bool averageBlocksBasedOnStdWiener;
		float averageBlocksBasedOnStdFactor;

		//Adaptivity
		bool meanAdaptiveThresholding;
		float meanAdaptiveThresholdingFactor;
		float meanAdaptiveThresholdingPower;

		//Multi-Threading
		index_t numThreadsBlockMatching;
		index_t numThreadsDenoising;

		void init2defaults(std::vector<float> smoothness, bool preview = false)
		{
			disableWienerFilter = false;

			stdDeviation = smoothness;

			if (preview)
			{
				stepSizeCols = 8;
				stepSizeRows = 8;
				searchWindowSize = 16;
				numPatchesPerBlockCollaborative = 16;
				numPatchesPerBlockWiener = 16;
			}
			else
			{
				stepSizeCols = 3;
				stepSizeRows = 3;
				searchWindowSize = 16;
				numPatchesPerBlockCollaborative = 16;
				numPatchesPerBlockWiener = 32;
			}

			//Block Matching
			templateMatchingMaxAllowedPatchDistance = 0.01f;
			templateMatchingNorm = 2;
			templateMatchingNumChannels = 3;

			patchSize = 8;

			usePatchWeighting = true;

			//Statistical Extensions
			averageBlocksBasedOnStdCollaborative = false;
			averageBlocksBasedOnStdWiener = false;
			averageBlocksBasedOnStdFactor = 0.0f;

			//Adaptivity
			meanAdaptiveThresholding = false;
			meanAdaptiveThresholdingFactor = 0.0f;

			//Multi-Threading
			numThreadsBlockMatching = 1;
			numThreadsDenoising = 1;
		}

		void enableBlockStatisticalAveraging(float factor)
		{
			averageBlocksBasedOnStdCollaborative = true;
			averageBlocksBasedOnStdWiener = false;
			averageBlocksBasedOnStdFactor = factor;
		}

		void enableMeanAdaptiveThresholding(float factor, float power)
		{
			meanAdaptiveThresholding = true;
			meanAdaptiveThresholdingFactor = factor;
			meanAdaptiveThresholdingPower = power;
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
			numPatchesPerBlockCollaborative = 16;
			numPatchesPerBlockWiener = 16;
		}

		void productionQuality()
		{
			stepSizeCols = 3;
			stepSizeRows = 3;
			searchWindowSize = 32;
			numPatchesPerBlockCollaborative = 16;
			numPatchesPerBlockWiener = 32;
		}
	};
}