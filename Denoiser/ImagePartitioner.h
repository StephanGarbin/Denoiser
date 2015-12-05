#pragma once

#include "Image.h"
#include "Rectangle.h"
#include "common.h"
#include "BM3DSettings.h"

namespace Denoise
{
	class ImagePartitioner
	{
	public:
		ImagePartitioner(const Image* image);
		~ImagePartitioner();

		void createPartitionScanlines(index_t numSegments, index_t& totalNumGeneratedBlocks,
			index_t patchSize, index_t stepSizeRows, index_t stepSizeCols);

		void createPartitionRectangles(index_t numSegments);

		inline index_t numSegments() const { return m_segments.size(); }

		inline const Rectangle& getSegment(index_t idx) const { return m_segments[idx]; }

		inline const std::vector<Rectangle>& getSegments() const { return m_segments; }

		inline const index_t getStartIdx(index_t idx) const  { return m_startIndices[idx]; }

	private:
		const Image* m_image;
		std::vector<Rectangle> m_segments;
		std::vector<index_t> m_startIndices;
	};
}

