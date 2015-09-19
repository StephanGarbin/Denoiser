#include "ImageNLMeansProcessor.h"

#include "ImageBlockProcessor.h"
#include "BufferAggregator.h"

#include "ImagePatch.h"
#include "Rectangle.h"

namespace Denoise
{

	ImageNLMeansProcessor::ImageNLMeansProcessor(Image* image) : m_image(image), m_buffer(image->fullDimension(), image->numChannels())
	{
		m_blockMatchingProcessed = false;
	}


	ImageNLMeansProcessor::~ImageNLMeansProcessor()
	{

	}

	void ImageNLMeansProcessor::process(const NLMeansSettings& settings, bool processMatching)
	{
		//lets remember last settings for future reference
		m_settings = settings;

		//1. Block Matching
		if (processMatching)
		{
			if (!m_blockMatchingProcessed)
			{
				std::cout << "NLMEANS ERROR: Block Matching must be processed at least once before" << std::endl;
				return;
			}

			ImageBlockProcessor blockProcessor(*m_image);

			ImagePatch patchTemplate(0, 0, m_settings.patchSize, m_settings.patchSize);
			Rectangle matchRegion(0, m_image->width(), 0, m_image->height());

			blockProcessor.computeNMostSimilar(patchTemplate, matchRegion,
				m_settings.stepSizeRows, m_settings.stepSizeCols,
				m_settings.searchWindowSize, m_settings.searchWindowSize,
				settings.numPatchesPerBlock, settings.maxAllowedPatchDistance,
				2, m_matchedBlocks);
		}

		//2. Process Blocks
		for (size_t row = 0; row < m_image->height() - m_settings.patchSize; row += m_settings.stepSizeRows)
		{
			for (size_t col = 0; col < m_image->width() - m_settings.patchSize; col += m_settings.stepSizeCols)
			{

			}
		}
	}
}
