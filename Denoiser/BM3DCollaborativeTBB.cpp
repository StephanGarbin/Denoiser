#include "BM3DCollaborativeTBB.h"

#include "BM3DCollaborativeFilterKernel.h"
#include "Statistics.h"

#include <iostream>

namespace Denoise
{

	BM3DCollaborativeTBB::BM3DCollaborativeTBB(Image* image, BufferAggregator& buffer,
		const BM3DSettings& settings, const std::vector<std::vector<IDX2> >& matchedBlocks,
		const ImagePatch& patchTemplate,
		TBB_MUTEX_TYPE& mutex,
		BM3DCollaborativeFilterKernel& kernel)
		: m_image(image), m_buffer(buffer), m_settings(settings),
		m_matchedBlocks(matchedBlocks), m_patchTemplate(patchTemplate),
		m_mutex(mutex),
		m_kernel(kernel)
	{

	}


	BM3DCollaborativeTBB::~BM3DCollaborativeTBB()
	{
	}

	void BM3DCollaborativeTBB::operator()(const tbb::blocked_range<size_t>& r) const
	{
		float* rawImageBlock = new float[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlockCollaborative * 3];

		for (index_t i = r.begin(); i != r.end(); ++i)
		{
			index_t numValidPatches;
			std::vector<float> weights(3);

			m_image->cpy2Block3d(m_matchedBlocks[i], rawImageBlock, m_patchTemplate, -3, numValidPatches);

			if (numValidPatches < 1)
			{
				continue;
			}

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
						m_kernel.processCollaborativeFilterMeanAdaptive(rawImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
					}
					else
					{
						m_kernel.processCollaborativeFilter(rawImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
					}
				}
			}
			else
			{
				if (m_settings.meanAdaptiveThresholding)
				{
					m_kernel.processCollaborativeFilterMeanAdaptive(rawImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
				}
				else
				{
					m_kernel.processCollaborativeFilter(rawImageBlock, numValidPatches, 3, weights, m_settings.stdDeviation);
				}
			}

			index_t sizePerChannel = numValidPatches * m_patchTemplate.width * m_patchTemplate.height;
			
			for (index_t channel = 0; channel < 3; ++channel)
			{
				TBB_MUTEX_TYPE::scoped_lock lock;
				lock.acquire(m_mutex);
				for (index_t depth = 0; depth < numValidPatches; ++depth)
				{
					for (index_t patchRow = 0; patchRow < m_patchTemplate.height; ++patchRow)
					{
						for (index_t patchCol = 0; patchCol < m_patchTemplate.width; ++patchCol)
						{
							m_buffer.addValueNumerator(channel, m_matchedBlocks[i][depth].row + patchRow,
								m_matchedBlocks[i][depth].col + patchCol,
								rawImageBlock[channel * sizePerChannel + depth * m_patchTemplate.width
								* m_patchTemplate.height + patchRow * m_patchTemplate.width + patchCol] * weights[channel]);

							m_buffer.addValueDenominator(channel, m_matchedBlocks[i][depth].row + patchRow,
								m_matchedBlocks[i][depth].col + patchCol, weights[channel]);
						}
					}
				}

				lock.release();
			}
		}

		delete[] rawImageBlock;
	}
}