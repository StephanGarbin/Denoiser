#pragma once
#include "Image.h"
#include "BM3DSettings.h"
#include "ImagePatch.h"
#include "Rectangle.h"
#include <vector>

namespace Denoise
{
	void computeBlockMatching(Image* image, const BM3DSettings& settings,
		const ImagePatch& templatePatch, const Rectangle& imageBlock,
		std::vector<std::vector<IDX2> >& matchedBlocks,
		index_t matchedBlocksAlreadyComputed,
		bool collaborative);
}
