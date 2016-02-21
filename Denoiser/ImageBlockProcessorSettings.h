#pragma once

#include "common.h"
#include "ImagePatch.h"
#include "Rectangle.h"

namespace Denoise
{
	struct ImageBlockProcessorSettings
	{
		ImagePatch templatePatch;
		Rectangle imageBlock;
		index_t stepSizeRows;
		index_t stepSizeCols;

		index_t windowSizeRows;
		index_t windowSizeCols;
		index_t maxSimilar;

		float maxDistance;

		int norm;
		index_t numChannelsToUse;

		index_t matchedBlocksAlreadyComputed;
		index_t numThreadsIntegralImageComputation;
		index_t numThreadsBlockMatching;

		bool useReferencePatchAdaptiveDistance;
		float referencePatchDistanceFactor;
	};
}