#include "BM3DImageBlockProcessor.h"
#include "ImageBlockProcessor.h"
#include "ImageBlockProcessorSettings.h"
#include "BufferAggregator.h"
#include "BM3DCollaborativeFilterKernel.h"
#include "BM3DWienerFilterKernel.h"
#include "BM3DImageBlockProcessorFunctions.h"

#include "ImagePartitioner.h"
#include "ImagePatch.h"
#include "Rectangle.h"

#include "common.h"

#include "DEBUG_HELPER.h"
#include "Statistics.h"

#include <algorithm>
#include <numeric>

#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/bind.hpp>

#include <tbb\tick_count.h>

namespace Denoise
{

	BM3DImageBlockProcessor::BM3DImageBlockProcessor(Image* image, Image* imageBasic, Image* imageResult)
		: m_image(image), m_imageBasic(imageBasic), m_imageResult(imageResult),
		m_buffer(image->fullDimension(), image->numChannels())
	{
		m_blockMatchingProcessed = false;
	}


	BM3DImageBlockProcessor::~BM3DImageBlockProcessor()
	{

	}

	void BM3DImageBlockProcessor::processBlockMatching(Image* image, bool collaborative)
	{
		m_matchedBlocks.clear();

		tbb::tick_count start = tbb::tick_count::now();

		ImagePatch patchTemplate(0, 0, m_settings.patchSize, m_settings.patchSize);

		Rectangle matchRegion(0, image->width(), image->height(), 0);

		m_matchedBlocks.resize((matchRegion.width() / m_settings.stepSizeCols + 1)
			* (matchRegion.height() / m_settings.stepSizeRows + 1));

		ImageBlockProcessor processor(*m_image);

		ImageBlockProcessorSettings blockMatchSettings;
		blockMatchSettings.templatePatch = patchTemplate;
		blockMatchSettings.imageBlock = matchRegion;
		blockMatchSettings.stepSizeRows = m_settings.stepSizeRows;
		blockMatchSettings.stepSizeCols = m_settings.stepSizeCols;
		blockMatchSettings.windowSizeRows = m_settings.searchWindowSize;
		blockMatchSettings.windowSizeCols = m_settings.searchWindowSize;
		if (collaborative)
		{
			blockMatchSettings.maxSimilar = m_settings.numPatchesPerBlockCollaborative;
		}
		else
		{
			blockMatchSettings.maxSimilar = m_settings.numPatchesPerBlockWiener;
		}
		blockMatchSettings.maxDistance = m_settings.templateMatchingMaxAllowedPatchDistance;
		blockMatchSettings.norm = m_settings.templateMatchingNorm;
		blockMatchSettings.numChannelsToUse = m_settings.templateMatchingNumChannels;
		blockMatchSettings.matchedBlocksAlreadyComputed = 0;
		blockMatchSettings.numThreadsIntegralImageComputation = m_settings.numThreadsBlockMatching;
		blockMatchSettings.numThreadsBlockMatching = m_settings.numThreadsBlockMatching;

		processor.computeNMostSimilar_PARALLEL_TBB(blockMatchSettings, m_settings, m_matchedBlocks);
		/*std::cout << "Computing Sequential Comparison..." << std::endl;
		std::vector<std::vector<IDX2> > matchedBlocks2;
		{
			Rectangle matchRegion(0, image->width(), image->height(), 0);

			ImageBlockProcessor blockProcessor(*image);

			matchedBlocks2.resize((matchRegion.width() / m_settings.stepSizeCols + 1)
				* (matchRegion.height() / m_settings.stepSizeRows + 1));

			ImageBlockProcessorSettings blockMatchSettings;
			blockMatchSettings.templatePatch = patchTemplate;
			blockMatchSettings.imageBlock = matchRegion;
			blockMatchSettings.stepSizeRows = m_settings.stepSizeRows;
			blockMatchSettings.stepSizeCols = m_settings.stepSizeCols;
			blockMatchSettings.windowSizeRows = m_settings.searchWindowSize;
			blockMatchSettings.windowSizeCols = m_settings.searchWindowSize;
			if (collaborative)
			{
				blockMatchSettings.maxSimilar = m_settings.numPatchesPerBlockCollaborative;
			}
			else
			{
				blockMatchSettings.maxSimilar = m_settings.numPatchesPerBlockWiener;
			}
			blockMatchSettings.maxDistance = m_settings.templateMatchingMaxAllowedPatchDistance;
			blockMatchSettings.norm = m_settings.templateMatchingNorm;
			blockMatchSettings.numChannelsToUse = m_settings.templateMatchingNumChannels;
			blockMatchSettings.matchedBlocksAlreadyComputed = 0;

			blockProcessor.computeNMostSimilar(blockMatchSettings, matchedBlocks2);
		}

		std::cout << "Comparing..." << std::endl;
		size_t counter = 0;
		for (index_t i = 0; i < m_matchedBlocks.size(); ++i)
		{
			bool matches = true;
			for (index_t b = 0; b < m_matchedBlocks[i].size(); ++b)
			{
				if (m_matchedBlocks[i][b].col != matchedBlocks2[i][b].col || m_matchedBlocks[i][b].row != matchedBlocks2[i][b].row)
				{
					matches = false;
					break;
				}
			}
			if (!matches)
			{
				++counter;
			}
		}

		std::cout << ((float)counter / (float)m_matchedBlocks.size()) * 100.0f << "% blocks are different" << std::endl;*/

		std::cout << "Finished Block Matching..." << std::endl;
		tbb::tick_count end = tbb::tick_count::now();
		std::cout << "Time: " << (end - start).seconds() << "s." << std::endl;
	}

	void BM3DImageBlockProcessor::process(const BM3DSettings& settings, bool processMatching)
	{
		//lets remember last settings for future reference
		m_settings = settings;

		//1. Block Matching
		ImagePatch patchTemplate(0, 0, m_settings.patchSize, m_settings.patchSize);

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
			processBlockMatching(m_image, true);
		}

		BM3DCollaborativeFilterKernel collaborativeKernel(m_settings);

		//2. Process Blocks
		{
			float* rawImageBlock = new float[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlockCollaborative * 3];

			for (index_t i = 0; i < m_matchedBlocks.size(); ++i)
			{
				index_t numValidPatches;
				std::vector<float> weights(3);

				m_image->cpy2Block3d(m_matchedBlocks[i], rawImageBlock, patchTemplate, -3, numValidPatches);

				if (numValidPatches < 1)
				{
					//std::cout << "Continuing..." << std::endl;
					continue;
				}
				//else
				//{
				//	for (index_t z = 0; z < numValidPatches; ++z)
				//	{
				//		std::cout << " [ " << m_matchedBlocks[i][z].row << ", " << m_matchedBlocks[i][z].col << ", " << m_matchedBlocks[i][z].distance << " ]; ";
				//	}
				//	std::cout << std::endl;
				//}

				//if (m_matchedBlocks[i][0].distance != 0.0f)
				//{
				//	std::cout << "Error: " << m_matchedBlocks[i][0].distance << ";; " << m_matchedBlocks[i][0].row << "; " << m_matchedBlocks[i][0].col << std::endl;
				//	std::cout << "ENext: " << m_matchedBlocks[i][1].distance << ";; " << m_matchedBlocks[i][1].row << "; " << m_matchedBlocks[i][1].col << std::endl;
				//	std::cout << "Size: " << m_matchedBlocks[i].size() << std::endl;
				//}

				if (m_settings.averageBlocksBasedOnStdCollaborative)
				{
					float blockStd = std::sqrt(calculateBlockVariance(rawImageBlock, m_settings.numPatchesPerBlockCollaborative, m_settings.patchSize, m_image->numChannels()));

					if (m_settings.meanAdaptiveThresholding)
					{
						float mean = calculateBlockMean(rawImageBlock, m_settings.numPatchesPerBlockCollaborative, m_settings.patchSize, m_image->numChannels());
						blockStd *= calculateMeanAdaptiveFactor(blockStd, mean, m_settings.meanAdaptiveThresholdingFactor);
					}

					if (blockStd < m_settings.stdDeviation * m_settings.averageBlocksBasedOnStdFactor)
					{
						setBlockToAveragePatch(rawImageBlock, m_settings.numPatchesPerBlockCollaborative, m_settings.patchSize, m_image->numChannels());

						for (index_t channel = 0; channel < weights.size(); ++channel)
						{
							weights[channel] = 1.0f;
						}
					}
					else
					{
						if (m_settings.meanAdaptiveThresholding)
						{
							collaborativeKernel.processCollaborativeFilterMeanAdaptive(rawImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
						}
						else
						{
							collaborativeKernel.processCollaborativeFilter(rawImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
						}
					}
				}
				else
				{
					if (m_settings.meanAdaptiveThresholding)
					{
						collaborativeKernel.processCollaborativeFilterMeanAdaptive(rawImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
					}
					else
					{
						collaborativeKernel.processCollaborativeFilter(rawImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
					}
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

			delete[] rawImageBlock;
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
					m_imageBasic->setPixel(channel, row, col, m_buffer.getValueResult(channel, row, col));
				}
			}
		}

		m_imageBasic->clamp(0.0f, 1.0f);

		if (!m_settings.disableWienerFilter)
		{
			//clear buffer
			m_buffer.clear();

			processBlockMatching(m_imageBasic, false);

			float* rawImageBlock = new float[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlockWiener * 3];

			float* estimateImageBlock = new float[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlockWiener * 3];

			BM3DWienerFilterKernel wienerKernel(m_settings);

			//2. Process Blocks
			for (index_t i = 0; i < m_matchedBlocks.size(); ++i)
			{
				index_t numValidPatches;
				std::vector<float> weights(3);

				//cpy BOTH blocks
				m_image->cpy2Block3d(m_matchedBlocks[i], rawImageBlock, patchTemplate, -3, numValidPatches);
				m_imageBasic->cpy2Block3d(m_matchedBlocks[i], estimateImageBlock, patchTemplate, -3, numValidPatches);

				if (numValidPatches < 1)
				{
					continue;
				}

				if (m_settings.averageBlocksBasedOnStdWiener)
				{
					float blockStd = std::sqrt(calculateBlockVariance(estimateImageBlock, m_settings.numPatchesPerBlockWiener, m_settings.patchSize, m_image->numChannels()));

					if (m_settings.meanAdaptiveThresholding)
					{
						float mean = calculateBlockMean(rawImageBlock, m_settings.numPatchesPerBlockWiener, m_settings.patchSize, m_image->numChannels());
						blockStd *= calculateMeanAdaptiveFactor(blockStd, mean, m_settings.meanAdaptiveThresholdingFactor);
					}

					if (blockStd < m_settings.stdDeviation * m_settings.averageBlocksBasedOnStdFactor)
					{
						setBlockToAveragePatch(rawImageBlock, m_settings.numPatchesPerBlockWiener, m_settings.patchSize, m_image->numChannels());

						for (index_t channel = 0; channel < weights.size(); ++channel)
						{
							weights[channel] = 1.0f;
						}
					}
					else
					{
						if (m_settings.meanAdaptiveThresholding)
						{
							wienerKernel.processWienerFilterMeanAdaptive(rawImageBlock, estimateImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
						}
						else
						{
							wienerKernel.processWienerFilter(rawImageBlock, estimateImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
						}
					}
				}
				else
				{
					if (m_settings.meanAdaptiveThresholding)
					{
						wienerKernel.processWienerFilterMeanAdaptive(rawImageBlock, estimateImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
					}
					else
					{
						wienerKernel.processWienerFilter(rawImageBlock, estimateImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
					}
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

			delete[] estimateImageBlock;
			delete[] rawImageBlock;
		}

		m_imageResult->clamp(0.0f, 1.0f);
	}

	void BM3DImageBlockProcessor::processCollaborativeFilter()
	{

	}

	void BM3DImageBlockProcessor::processWienerFilter()
	{

	}
}

