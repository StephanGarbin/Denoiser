#include "BM3DImageBlockProcessorFunctions.h"
#include "ImageBlockProcessor.h"
#include "ImageBlockProcessorFunctions.h"

namespace Denoise
{
	void computeBlockMatching(Image* image, const BM3DSettings& settings,
		const ImagePatch& templatePatch, const Rectangle& imageBlock,
		std::vector<std::vector<IDX2> >& matchedBlocks,
		index_t matchedBlocksAlreadyComputed,
		bool collaborative)
	{
		ImageBlockProcessor blockProcessor(*image);

		ImageBlockProcessorSettings blockMatchSettings;
		blockMatchSettings.templatePatch = templatePatch;
		blockMatchSettings.imageBlock = imageBlock;
		blockMatchSettings.stepSizeRows = settings.stepSizeRows;
		blockMatchSettings.stepSizeCols = settings.stepSizeCols;
		blockMatchSettings.windowSizeRows = settings.searchWindowSize;
		blockMatchSettings.windowSizeCols = settings.searchWindowSize;
		if (collaborative)
		{
			blockMatchSettings.maxSimilar = settings.numPatchesPerBlockCollaborative;
		}
		else
		{
			blockMatchSettings.maxSimilar = settings.numPatchesPerBlockWiener;
		}
		blockMatchSettings.maxDistance = settings.templateMatchingMaxAllowedPatchDistance;
		blockMatchSettings.norm = settings.templateMatchingNorm;
		blockMatchSettings.numChannelsToUse = settings.templateMatchingNumChannels;
		blockMatchSettings.matchedBlocksAlreadyComputed = matchedBlocksAlreadyComputed;
		blockMatchSettings.numThreadsBlockMatching = 1;
		blockMatchSettings.numThreadsIntegralImageComputation = 1;

		blockProcessor.computeNMostSimilar(blockMatchSettings, matchedBlocks);
	}
}
