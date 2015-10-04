#pragma once
#include "common.h"

namespace Denoise
{
	struct NLMeansSettings
	{
		float stdDeviation;
		float filteringParameter;

		index_t stepSizeCols;
		index_t stepSizeRows;

		index_t searchWindowSize;

		index_t numPatchesPerBlock;

		float maxAllowedPatchDistance;

		int patchSize;

		bool usePatchWeighting;
	};
}