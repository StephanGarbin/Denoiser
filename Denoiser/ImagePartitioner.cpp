#include "ImagePartitioner.h"

namespace Denoise
{

	ImagePartitioner::ImagePartitioner(const Image& image) : m_image(image)
	{

	}


	ImagePartitioner::~ImagePartitioner()
	{
		m_segments.clear();
	}

	void ImagePartitioner::createPartitionScanlines(size_t numSegments)
	{

	}

	void ImagePartitioner::createPartitionRectangles(size_t numSegments)
	{

	}

}
