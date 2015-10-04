#pragma once
#include "common.h"

namespace Denoise
{
	struct BM3DSettings
	{
		float stdDeviation;

		index_t stepSizeCols;
		index_t stepSizeRows;

		index_t searchWindowSize;

		index_t numPatchesPerBlock;

		float maxAllowedPatchDistance;

		int patchSize;

		bool usePatchWeighting;

		//Statistical Extensions
		bool averageBlocksBasedOnStd;
		float averageBlocksBasedOnStdFactor;
	};
}