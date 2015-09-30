#pragma once

namespace Denoise
{
	struct NLMeansSettings
	{
		float stdDeviation;
		float filteringParameter;

		size_t stepSizeCols;
		size_t stepSizeRows;

		size_t searchWindowSize;

		size_t numPatchesPerBlock;

		float maxAllowedPatchDistance;

		int patchSize;

		bool usePatchWeighting;
	};
}