#include "BM3DImageBlockProcessorBOOST.h"

namespace Denoise
{

	void processCollaborative(BM3DCollaborativeFunctor& processor, const std::pair<size_t, size_t>& range)
	{
		processor.operator()(range);
	}

	void processWiener(BM3DWienerFunctor& processor, const std::pair<size_t, size_t>& range)
	{
		processor.operator()(range);
	}

}