#include "BM3DImageBlockProcessor.h"
#include "ImageBlockProcessor.h"
#include "BufferAggregator.h"

#include "ImagePatch.h"
#include "Rectangle.h"

#include "common.h"

#include "DEBUG_HELPER.h"

#include <algorithm>

namespace Denoise
{

	BM3DImageBlockProcessor::BM3DImageBlockProcessor(Image* image, Image* imageResult) : m_image(image), m_imageResult(imageResult),
		m_buffer(image->fullDimension(), image->numChannels())
	{
		m_blockMatchingProcessed = false;
	}


	BM3DImageBlockProcessor::~BM3DImageBlockProcessor()
	{

	}

	void BM3DImageBlockProcessor::process(const BM3DSettings& settings, bool processMatching)
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
				std::cout << "BM3D ERROR: Block Matching must be processed at least once before" << std::endl;
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
				2, m_matchedBlocks, 3);

			std::cout << "Finished Block Matching..." << std::endl;
		}

		float* rawImageBlock = new float[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlock];

		//2. Process Blocks
		for (size_t channel = 0; channel < 3; ++channel)
		{
			for (size_t row = 1; row < m_image->height() - m_settings.patchSize; row += m_settings.stepSizeRows)
			{
				for (size_t col = 1; col < m_image->width() - m_settings.patchSize; col += m_settings.stepSizeCols)
				{
					size_t numValidPatches;

					m_image->cpy2Block3d(m_matchedBlocks[row * matchRegion.width() + col], rawImageBlock, patchTemplate, channel, numValidPatches);

					bm3dDEBUG(rawImageBlock, settings.stdDeviation);

					double weight = 1.0f;

					for (size_t depth = 0; depth < numValidPatches; ++depth)
					{
						for (size_t patchRow = 0; patchRow < patchTemplate.height; ++patchRow)
						{
							for (size_t patchCol = 0; patchCol < patchTemplate.width; ++patchCol)
							{
								m_buffer.addValueNumerator(channel, m_matchedBlocks[row * matchRegion.width() + col][depth].row + patchRow,
									m_matchedBlocks[row * matchRegion.width() + col][depth].col + patchCol,
									rawImageBlock[depth * patchTemplate.width * patchTemplate.height + patchRow * patchTemplate.width + patchCol]);

								m_buffer.addValueDenominator(channel, m_matchedBlocks[row * matchRegion.width() + col][depth].row + patchRow,
									m_matchedBlocks[row * matchRegion.width() + col][depth].col + patchCol, weight);
							}
						}
					}
				}
			}
		}

		//divide buffers
		m_buffer.divideBuffers();

		//set result image
		for (size_t channel = 0; channel < 3; ++channel)
		{
			for (size_t row = 0; row < m_image->height(); row += m_settings.stepSizeRows)
			{
				for (size_t col = 0; col < m_image->width(); col += m_settings.stepSizeCols)
				{
					m_imageResult->setPixel(channel, row, col, m_buffer.getValueResult(channel, row, col));
				}
			}
		}

		delete[] rawImageBlock;
	}
}

