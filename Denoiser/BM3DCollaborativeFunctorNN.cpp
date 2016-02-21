#include "BM3DCollaborativeFunctorNN.h"

#include "BM3DCollaborativeFilterKernel.h"
#include "Statistics.h"

#include "TRANSFORM_DOMAIN_FORMATS.h"

#include <iostream>
#include <numeric>

namespace Denoise
{

	BM3DCollaborativeFunctorNN::BM3DCollaborativeFunctorNN(Image* image, BufferAggregator& buffer,
		const BM3DSettings& settings, const std::vector<std::vector<IDX2> >& matchedBlocks,
		const ImagePatch& patchTemplate,
		MUTEX_TYPE& mutex)
		: m_image(image), m_buffer(buffer), m_settings(settings),
		m_matchedBlocks(matchedBlocks), m_patchTemplate(patchTemplate),
		m_mutex(mutex)
	{
		MUTEX_TYPE::scoped_lock lock(m_mutex);
		m_kernel = std::make_shared<BM3DCollaborativeFilterKernel>(m_settings);
	}


	BM3DCollaborativeFunctorNN::~BM3DCollaborativeFunctorNN()
	{
	}

	BM3DCollaborativeFunctorNN::BM3DCollaborativeFunctorNN(const BM3DCollaborativeFunctorNN& other) :
		m_image(other.m_image), m_buffer(other.m_buffer),
		m_settings(other.m_settings), m_patchTemplate(other.m_patchTemplate),
		m_matchedBlocks(other.m_matchedBlocks), m_mutex(other.m_mutex)
	{
		MUTEX_TYPE::scoped_lock lock(m_mutex);
		m_kernel = std::make_shared<BM3DCollaborativeFilterKernel>(m_settings);
	}

	void BM3DCollaborativeFunctorNN::operator()(const std::pair<size_t, size_t>& r, float* destination, int* destinationIdxs,
		bool loadBlocks) const
	{
		DOMAIN_FORMAT* rawImageBlock = new DOMAIN_FORMAT[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlockCollaborative * 3];

		for (size_t i = r.first; i != r.second; ++i)
		{
			index_t numValidPatches;
			std::vector<DOMAIN_FORMAT> weights(3);

			m_image->cpy2Block3d(m_matchedBlocks[i], rawImageBlock, m_patchTemplate, -3, numValidPatches);

			if (numValidPatches < 1)
			{
				continue;
			}
			
			//Save blocks as required by the IDX prescription
			if (destinationIdxs[i] > 0)
			{
				m_kernel->processCollaborativeFilterFrequencyData(rawImageBlock, numValidPatches,
					3, weights, m_settings.stdDeviation,
					destination, destinationIdxs[i], !loadBlocks);
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
									m_matchedBlocks[i][depth].col + patchCol, (float)weights[channel]);
							}
						}
					}
				}
			}
		}

		delete[] rawImageBlock;
	}
}
