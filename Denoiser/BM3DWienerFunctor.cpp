#include "BM3DWienerFunctor.h"

#include "Statistics.h"
#include "TRANSFORM_DOMAIN_FORMATS.h"

#include <iostream>
#include <numeric>

namespace Denoise
{

	BM3DWienerFunctor::BM3DWienerFunctor(Image* image, Image* basicImage, BufferAggregator& buffer,
		const BM3DSettings& settings, const std::vector<std::vector<IDX2> >& matchedBlocks,
		const ImagePatch& patchTemplate,
		MUTEX_TYPE& mutex)
		: m_image(image), m_basicImage(basicImage), m_buffer(buffer), m_settings(settings),
		m_matchedBlocks(matchedBlocks), m_patchTemplate(patchTemplate),
		m_mutex(mutex)
	{
		MUTEX_TYPE::scoped_lock lock(m_mutex);
		m_kernel = std::make_shared<BM3DWienerFilterKernel>(m_settings);
	}


	BM3DWienerFunctor::~BM3DWienerFunctor()
	{
	}

	BM3DWienerFunctor::BM3DWienerFunctor(const BM3DWienerFunctor& other) :
		m_image(other.m_image), m_basicImage(other.m_basicImage), m_buffer(other.m_buffer),
		m_settings(other.m_settings), m_patchTemplate(other.m_patchTemplate),
		m_matchedBlocks(other.m_matchedBlocks), m_mutex(other.m_mutex)
	{
		MUTEX_TYPE::scoped_lock lock(m_mutex);
		m_kernel = std::make_shared<BM3DWienerFilterKernel>(m_settings);
	}

	void BM3DWienerFunctor::operator()(const std::pair<size_t, size_t>& r) const
	{
		DOMAIN_FORMAT* rawImageBlock = new DOMAIN_FORMAT[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlockWiener * 3];

		DOMAIN_FORMAT* estimateImageBlock = new DOMAIN_FORMAT[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlockWiener * 3];

		//2. Process Blocks
		for (size_t i = r.first; i != r.second; ++i)
		{
			index_t numValidPatches;
			std::vector<DOMAIN_FORMAT> weights(3);

			//cpy BOTH blocks
			m_image->cpy2Block3d(m_matchedBlocks[i], rawImageBlock, m_patchTemplate, -3, numValidPatches);
			m_basicImage->cpy2Block3d(m_matchedBlocks[i], estimateImageBlock, m_patchTemplate, -3, numValidPatches);

			if (numValidPatches < 1)
			{
				continue;
			}

			if (m_settings.averageBlocksBasedOnStdWiener)
			{
				DOMAIN_FORMAT blockStd = std::sqrt(calculateBlockVariance(estimateImageBlock, m_settings.numPatchesPerBlockWiener, m_settings.patchSize, m_image->numChannels()));

				if (m_settings.meanAdaptiveThresholding)
				{
					DOMAIN_FORMAT mean = calculateBlockMean(rawImageBlock, m_settings.numPatchesPerBlockWiener, m_settings.patchSize, m_image->numChannels());
					blockStd *= calculateMeanAdaptiveFactor(blockStd, mean, m_settings.meanAdaptiveThresholdingFactor, m_settings.meanAdaptiveThresholdingPower);
				}

				DOMAIN_FORMAT meanStdDeviation = std::accumulate(m_settings.stdDeviation.begin(), m_settings.stdDeviation.end(), 0.0f)
					/ m_settings.stdDeviation.size();

				if (blockStd < meanStdDeviation * m_settings.averageBlocksBasedOnStdFactor)
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
						m_kernel->processWienerFilterMeanAdaptive(rawImageBlock, estimateImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
					}
					else
					{
						m_kernel->processWienerFilter(rawImageBlock, estimateImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
					}
				}
			}
			else
			{
				if (m_settings.meanAdaptiveThresholding)
				{
					m_kernel->processWienerFilterMeanAdaptive(rawImageBlock, estimateImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
				}
				else
				{
					m_kernel->processWienerFilter(rawImageBlock, estimateImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
				}
			}

			index_t sizePerChannel = numValidPatches * m_patchTemplate.width * m_patchTemplate.height;

			{
				MUTEX_TYPE::scoped_lock lock(m_mutex);
				for (index_t channel = 0; channel < 3; ++channel)
				{
					for (index_t depth = 0; depth < numValidPatches; ++depth)
					{
						for (index_t patchRow = 0; patchRow < m_patchTemplate.height; ++patchRow)
						{
							for (index_t patchCol = 0; patchCol < m_patchTemplate.width; ++patchCol)
							{
								m_buffer.addValueNumerator(channel, m_matchedBlocks[i][depth].row + patchRow,
									m_matchedBlocks[i][depth].col + patchCol,
									(float)(rawImageBlock[channel * sizePerChannel + depth * m_patchTemplate.width
									* m_patchTemplate.height + patchRow * m_patchTemplate.width + patchCol] * weights[channel]));

								m_buffer.addValueDenominator(channel, m_matchedBlocks[i][depth].row + patchRow,
									(float)m_matchedBlocks[i][depth].col + patchCol, weights[channel]);
							}
						}
					}
				}
			}
		}

		delete[] estimateImageBlock;
		delete[] rawImageBlock;
	}
}