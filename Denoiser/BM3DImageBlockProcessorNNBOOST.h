#pragma once

#include <utility>

#include "BM3DCollaborativeFunctorNN.h"
#include "BM3DWienerFunctor.h"


namespace Denoise
{

	void processCollaborativeNN(BM3DCollaborativeFunctorNN& processor, const std::pair<size_t, size_t>& range,
		float* destination, int* destinationIdxs,
		bool loadBlocks,
		float* untransformedRef = nullptr);

	void processWienerNN(BM3DWienerFunctor& processor, const std::pair<size_t, size_t>& range);

}