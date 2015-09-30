#pragma once

namespace Denoise
{
	struct BM3DSettings
	{
		float stdDeviation;

		size_t stepSizeCols;
		size_t stepSizeRows;

		size_t searchWindowSize;

		size_t numPatchesPerBlock;

		float maxAllowedPatchDistance;

		int patchSize;

		bool usePatchWeighting;
	};
}