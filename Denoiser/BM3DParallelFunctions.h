#pragma once

#include "BM3DImageBlockProcessor.h"
#include "Rectangle.h"
#include "common.h"

namespace Denoise
{
	void bm3dCollaborativeKernel(BM3DImageBlockProcessor* processor, index_t threadIdx, const Rectangle& region);
	void bm3dWienerKernel(BM3DImageBlockProcessor* processor, index_t threadIdx, const Rectangle& region);
}

