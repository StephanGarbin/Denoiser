#pragma once

#include "BM3DImageBlockProcessor.h"
#include "Rectangle.h"

namespace Denoise
{
	void bm3dCollaborativeKernel(BM3DImageBlockProcessor* processor, size_t threadIdx, const Rectangle& region);
	void bm3dWienerKernel(BM3DImageBlockProcessor* processor, size_t threadIdx, const Rectangle& region);
}

