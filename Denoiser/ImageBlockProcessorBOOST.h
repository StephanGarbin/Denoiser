#pragma once

#include "IntergralImageComputerTBB.h"
#include "BlockMatchingComputerTBB.h"

namespace Denoise
{

	void computeIntegralImages(IntergralImageComputerTBB& functor, const std::pair<size_t, size_t>& range);

	void computeBlockMatching(BlockMatchingComputerTBB& functor, const std::pair<size_t, size_t>& range);

}