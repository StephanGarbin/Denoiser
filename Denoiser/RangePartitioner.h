#pragma once

#include <vector>
#include <utility>
#include <cstddef>

namespace Denoise
{

	class RangePartitioner
	{
	public:
		RangePartitioner();
		~RangePartitioner();

		std::pair<size_t, size_t>& getSegment(size_t idx);

		size_t numSegments();

		void createPartition(size_t size, size_t numSegments);

	private:
		std::vector<std::pair<size_t, size_t> > m_segments;
	};
}


