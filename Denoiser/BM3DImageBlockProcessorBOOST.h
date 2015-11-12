#pragma once

#include <utility>

#include "BM3DCollaborativeFunctor.h"
#include "BM3DWienerFunctor.h"


namespace Denoise
{

	void processCollaborative(BM3DCollaborativeFunctor& processor, const std::pair<size_t, size_t>& range);

	void processWiener(BM3DWienerFunctor& processor, const std::pair<size_t, size_t>& range);

}