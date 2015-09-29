#include "ImageNLMeansProcessor.h"

#include "ImageBlockProcessor.h"
#include "BufferAggregator.h"

#include "ImagePatch.h"
#include "Rectangle.h"

#include "common.h"

#include <algorithm>

namespace Denoise
{

	ImageNLMeansProcessor::ImageNLMeansProcessor(Image* image, Image* imageResult) : m_image(image), m_imageResult(imageResult),
		m_buffer(image->fullDimension(), image->numChannels())
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

		ImagePatch patchTemplate(0, 0, m_settings.patchSize, m_settings.patchSize);
		Rectangle matchRegion(0, m_image->width(), m_image->height(), 0);

		if (!processMatching)
		{
			if (!m_blockMatchingProcessed)
			{
				std::cout << "NLMEANS ERROR: Block Matching must be processed at least once before" << std::endl;
				return;
			}
		}
		else
		{
			ImageBlockProcessor blockProcessor(*m_image);

			blockProcessor.computeNMostSimilar(patchTemplate, matchRegion,
				m_settings.stepSizeRows, m_settings.stepSizeCols,
				m_settings.searchWindowSize, m_settings.searchWindowSize,
				settings.numPatchesPerBlock, settings.maxAllowedPatchDistance,
				2, m_matchedBlocks);

			std::cout << "Finished Block Matching..." << std::endl;
		}

		float* rawImageBlock = new float[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlock];

		//2. Process Blocks
		for (size_t row = 1; row < m_image->height() - m_settings.patchSize; row += m_settings.stepSizeRows)
		{
			for (size_t col = 1; col < m_image->width() - m_settings.patchSize; col += m_settings.stepSizeCols)
			{
				size_t numValidPatches;

				m_image->cpy2Block3d(m_matchedBlocks[row * matchRegion.width() + col], rawImageBlock, patchTemplate, 0, numValidPatches);

				for (size_t depth = 0; depth < numValidPatches; ++depth)
				{
					float weight = 1.0f;
					
					if (m_settings.usePatchWeighting)
					{
						weight = std::exp(
							-std::max(std::pow(m_matchedBlocks[row * matchRegion.width() + col][depth].distance, 2) - 2.0f * m_settings.variance, 0.0f)
							/ (10.0f * std::sqrt(m_settings.variance)));
					}

					for (size_t patchRow = 0; patchRow < patchTemplate.height; ++patchRow)
					{
						for (size_t patchCol = 0; patchCol < patchTemplate.width; ++patchCol)
						{
							m_buffer.addValueNumerator(0, row + patchRow, col + patchCol,
								rawImageBlock[depth * patchTemplate.width * patchTemplate.height + patchRow * patchTemplate.width + patchCol]);
							m_buffer.addValueDenominator(0, row + patchRow, col + patchCol, weight);
						}
					}
				}
			}
		}

		//divide buffers
		m_buffer.divideBuffers();

		//set result image
		for (size_t row = 0; row < m_image->height(); row += m_settings.stepSizeRows)
		{
			for (size_t col = 0; col < m_image->width(); col += m_settings.stepSizeCols)
			{
				m_imageResult->setPixel(0, row, col, m_buffer.getValueResult(0, row, col));
			}
		}

		delete[] rawImageBlock;
	}
}
