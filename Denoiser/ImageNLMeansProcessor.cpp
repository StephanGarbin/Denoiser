#include "ImageNLMeansProcessor.h"

#include "ImageBlockProcessor.h"
#include "BufferAggregator.h"

#include "ImagePatch.h"
#include "Rectangle.h"

#include "common.h"

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
		for (size_t row = 10; row < m_image->height() - m_settings.patchSize - 10; row += m_settings.stepSizeRows)
		{
			for (size_t col = 10; col < m_image->width() - m_settings.patchSize - 10; col += m_settings.stepSizeCols)
			{
				m_image->cpy2Block3d(m_matchedBlocks[row * matchRegion.width() + col], rawImageBlock, patchTemplate, 0);

				for (size_t patchRow = 0; patchRow < patchTemplate.height; ++patchRow)
				{
					for (size_t patchCol = 0; patchCol < patchTemplate.width; ++patchCol)
					{
						float sum = 0.0f;
						for (size_t depth = 0; depth < m_settings.numPatchesPerBlock; ++depth)
						{
							sum += rawImageBlock[depth * patchTemplate.width * patchTemplate.height + patchRow * patchTemplate.width + patchCol];
						}

						sum /= (float)(m_settings.numPatchesPerBlock);
						m_buffer.addValueNumerator(0, row + patchRow, col + patchCol, sum);
						m_buffer.addValueDenominator(0, row + patchRow, col + patchCol, 1.0f);
					}
				}
			}
		}

		//divide buffers
		m_buffer.divideBuffers();

		//set result image
		for (size_t row = 0; row < m_image->height() - m_settings.patchSize; row += m_settings.stepSizeRows)
		{
			for (size_t col = 0; col < m_image->width() - m_settings.patchSize; col += m_settings.stepSizeCols)
			{
				m_imageResult->setPixel(0, row, col, m_buffer.getValueResult(0, row, col));
			}
		}

		delete[] rawImageBlock;
	}
}
