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

	void ImagePartitioner::createPartitionScanlines(index_t numSegments)
	{

	}

	void ImagePartitioner::createPartitionRectangles(index_t numSegments)
	{

	}

}
