#pragma once

#include "Image.h"
#include "Rectangle.h"
#include "common.h"

namespace Denoise
{
	class ImagePartitioner
	{
	public:
		ImagePartitioner(const Image& image);
		~ImagePartitioner();

		void createPartitionScanlines(index_t numSegments);

		void createPartitionRectangles(index_t numSegments);

		inline index_t numSegments() const { return m_segments.size(); }

		inline const Rectangle& getSegment(index_t idx) const { return m_segments[idx]; };

	private:
		const Image& m_image;
		std::vector<Rectangle> m_segments;
	};
}

