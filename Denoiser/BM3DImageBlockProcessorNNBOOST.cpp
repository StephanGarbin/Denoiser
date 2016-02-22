#include "BM3DImageBlockProcessorNNBOOST.h"

namespace Denoise
{

	void processCollaborativeNN(BM3DCollaborativeFunctorNN& processor, const std::pair<size_t, size_t>& range,
		float* destination, int* destinationIdxs,
		bool loadBlocks,
		float* untransformedRef)
	{
		processor.operator()(range, destination, destinationIdxs, loadBlocks, untransformedRef);
	}

	void processWienerNN(BM3DWienerFunctor& processor, const std::pair<size_t, size_t>& range)
	{
		processor.operator()(range);
	}

}