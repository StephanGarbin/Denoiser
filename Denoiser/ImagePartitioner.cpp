#include "ImagePartitioner.h"

#include <algorithm>
#include <numeric>

namespace Denoise
{

	ImagePartitioner::ImagePartitioner(const Image* image)
		: m_image(image)
	{

	}


	ImagePartitioner::~ImagePartitioner()
	{
		m_segments.clear();
		m_startIndices.clear();
	}

	void ImagePartitioner::createPartitionScanlines(index_t numSegments, index_t& totalNumGeneratedBlocks,
		index_t patchSize, index_t stepSizeRows, index_t stepSizeCols)
	{
		m_segments.resize(numSegments);
		m_startIndices.resize(numSegments + 1);

		ImagePatch patchTemplate(0, 0, patchSize, patchSize);

		index_t scaledHeight = m_image->height() / stepSizeRows;
		index_t scaledWidth = m_image->width() / stepSizeCols;

		index_t scaledScanLineHeight = std::ceil((float)scaledHeight / (float)numSegments);

		//1. Determine Scanlines
		m_startIndices[0] = 0;

		for (index_t t = 0; t < numSegments; ++t)
		{
			index_t scaledStart = t * scaledScanLineHeight;
			index_t scaledEnd = (t + 1) * scaledScanLineHeight;

			index_t start = scaledStart * stepSizeRows;
			index_t end = std::min<index_t>(scaledEnd * stepSizeRows, m_image->height());


			m_segments[t] = Rectangle(0, m_image->width(), end, start);
			m_startIndices[t] = scaledScanLineHeight * scaledStart;
		}

		totalNumGeneratedBlocks = std::accumulate(m_startIndices.begin(), m_startIndices.end(), 0);

		std::partial_sum<>(m_startIndices.begin(), m_startIndices.end(), m_startIndices.begin());
	}

	void ImagePartitioner::createPartitionRectangles(index_t numSegments)
	{

	}

}
