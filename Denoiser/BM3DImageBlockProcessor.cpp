#include "BM3DImageBlockProcessor.h"
#include "ImageBlockProcessor.h"
#include "BufferAggregator.h"
#include "BM3DCollaborativeFilterKernel.h"

#include "ImagePatch.h"
#include "Rectangle.h"

#include "common.h"

#include "DEBUG_HELPER.h"
#include "Statistics.h"

#include <algorithm>
//#include <tbb\tick_count.h>

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
			//tbb::tick_count start = tbb::tick_count::now();
			ImageBlockProcessor blockProcessor(*m_image);

			blockProcessor.computeNMostSimilar(patchTemplate, matchRegion,
				m_settings.stepSizeRows, m_settings.stepSizeCols,
				m_settings.searchWindowSize, m_settings.searchWindowSize,
				settings.numPatchesPerBlock, settings.maxAllowedPatchDistance,
				2, m_matchedBlocks, 3);

			//std::cout << "Finished Block Matching..." << std::endl;
			//tbb::tick_count end = tbb::tick_count::now();
			//std::cout << "Time: " << (end - start).seconds() << "s." << std::endl;
		}

		float* rawImageBlock = new float[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlock * 3];

		BM3DCollaborativeFilterKernel collaborativeKernel(m_settings);

		//2. Process Blocks
		for (index_t i = 0; i < m_matchedBlocks.size(); ++i)
		{
			index_t numValidPatches;
			std::vector<float> weights(3);

			m_image->cpy2Block3d(m_matchedBlocks[i], rawImageBlock, patchTemplate, -3, numValidPatches);

			if (numValidPatches < 1)
			{
				continue;
			}

			if (m_settings.averageBlocksBasedOnStd)
			{
				float blockStd = calculateBlockVariance(rawImageBlock, m_settings.numPatchesPerBlock, m_settings.patchSize);

				if (blockStd < m_settings.stdDeviation * m_settings.averageBlocksBasedOnStdFactor)
				{
					setBlockToAveragePatch(rawImageBlock, m_settings.numPatchesPerBlock, m_settings.patchSize);

					for (index_t channel = 0; channel < weights.size(); ++channel)
					{
						weights[channel] = 1.0f;
					}
				}
				else
				{
					collaborativeKernel.processCollaborativeFilter(rawImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
				}
			}
			else
			{
				collaborativeKernel.processCollaborativeFilter(rawImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
			}

			index_t sizePerChannel = numValidPatches * patchTemplate.width * patchTemplate.height;

			for (index_t channel = 0; channel < 3; ++channel)
			{
				for (index_t depth = 0; depth < numValidPatches; ++depth)
				{
					for (index_t patchRow = 0; patchRow < patchTemplate.height; ++patchRow)
					{
						for (index_t patchCol = 0; patchCol < patchTemplate.width; ++patchCol)
						{
							m_buffer.addValueNumerator(channel, m_matchedBlocks[i][depth].row + patchRow,
								m_matchedBlocks[i][depth].col + patchCol,
								rawImageBlock[channel * sizePerChannel + depth * patchTemplate.width * patchTemplate.height + patchRow * patchTemplate.width + patchCol] * weights[channel]);

							m_buffer.addValueDenominator(channel, m_matchedBlocks[i][depth].row + patchRow,
								m_matchedBlocks[i][depth].col + patchCol, weights[channel]);
						}
					}
				}
			}
		}

		//divide buffers
		m_buffer.divideBuffers();

		//set result image
		for (index_t channel = 0; channel < 3; ++channel)
		{
			for (index_t row = 0; row < m_image->height(); row += 1)
			{
				for (index_t col = 0; col < m_image->width(); col += 1)
				{
					m_imageResult->setPixel(channel, row, col, m_buffer.getValueResult(channel, row, col));
				}
			}
		}

		delete[] rawImageBlock;

		m_imageResult->clamp(0.0f, 1.0f);
	}

	void BM3DImageBlockProcessor::processCollaborativeFilter()
	{

	}

	void BM3DImageBlockProcessor::processWienerFilter()
	{

	}
}

