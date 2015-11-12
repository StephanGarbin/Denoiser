#pragma once

#include <vector>
#include <utility>

namespace Denoise
{

	class RangePartitioner
	{
	public:
		RangePartitioner();
		~RangePartitioner();

		inline const std::pair<size_t, size_t>& getSegment(size_t idx){ return m_segments[idx]; }

		inline size_t numSegments(){ return m_segments.size(); }

		void createPartition(size_t size, size_t numSegments);

	private:
		std::vector<std::pair<size_t, size_t> > m_segments;
	};
}

