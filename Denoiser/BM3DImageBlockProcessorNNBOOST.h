#pragma once

#include <utility>

#include "BM3DCollaborativeFunctorNN.h"
#include "BM3DWienerFunctor.h"


namespace Denoise
{

	void processCollaborative(BM3DCollaborativeFunctorNN& processor, const std::pair<size_t, size_t>& range,
		float* destination, int* destinationIdxs,
		bool loadBlocks);

	void processWiener(BM3DWienerFunctor& processor, const std::pair<size_t, size_t>& range);

}