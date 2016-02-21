#include "BM3DImageBlockProcessorNNBOOST.h"

namespace Denoise
{

	void processCollaborative(BM3DCollaborativeFunctorNN& processor, const std::pair<size_t, size_t>& range,
		float* destination, int* destinationIdxs,
		bool loadBlocks)
	{
		processor.operator()(range, destination, destinationIdxs, loadBlocks);
	}

	void processWiener(BM3DWienerFunctor& processor, const std::pair<size_t, size_t>& range)
	{
		processor.operator()(range);
	}

}