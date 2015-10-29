#pragma once
#include "common.h"

namespace Denoise
{
	struct BM3DSettings
	{
		bool disableWienerFilter;

		float stdDeviation;

		index_t stepSizeCols;
		index_t stepSizeRows;

		index_t searchWindowSize;

		index_t numPatchesPerBlockCollaborative;
		index_t numPatchesPerBlockWiener;

		//Block Matching
		float templateMatchingMaxAllowedPatchDistance;
		int templateMatchingNorm;
		index_t templateMatchingNumChannels;

		int patchSize;

		bool usePatchWeighting;

		//Statistical Extensions
		bool averageBlocksBasedOnStd;
		float averageBlocksBasedOnStdFactor;

		//Multi-Threading
		index_t numThreadsBlockMatching;
	};
}