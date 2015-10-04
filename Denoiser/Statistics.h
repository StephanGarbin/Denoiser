#pragma once

#include "common.h"

namespace Denoise
{
	float calculateBlockVariance(float* block, index_t numPatches, index_t patchSize);
	void setBlockToAveragePatch(float* block, index_t numPatches, index_t patchSize);
}

