#include "RangePartitioner.h"

namespace Denoise
{

	RangePartitioner::RangePartitioner()
	{
	}


	RangePartitioner::~RangePartitioner()
	{
	}

	void RangePartitioner::createPartition(size_t size, size_t numSegments)
	{
		size_t segmentSize = size / numSegments;

		size_t remainder = size - segmentSize * numSegments;

		if (remainder / numSegments > 0)
		{
			segmentSize += remainder / numSegments;
		}

		remainder = size - segmentSize * numSegments;

		for (size_t i = 0; i < numSegments - 1; ++i)
		{
			m_segments.push_back(std::pair<size_t, size_t>(segmentSize * i, segmentSize * (i + 1)));
		}

		m_segments.push_back(std::pair<size_t, size_t>((numSegments - 1) * segmentSize, size));
	}

}
