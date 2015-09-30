#pragma once

#include "Image.h"
#include "Rectangle.h"

namespace Denoise
{
	class ImagePartitioner
	{
	public:
		ImagePartitioner(const Image& image);
		~ImagePartitioner();

		void createPartitionScanlines(size_t numSegments);

		void createPartitionRectangles(size_t numSegments);

		inline size_t numSegments() const { return m_segments.size(); }

		inline const Rectangle& getSegment(size_t idx) const { return m_segments[idx]; };

	private:
		const Image& m_image;
		std::vector<Rectangle> m_segments;
	};
}

