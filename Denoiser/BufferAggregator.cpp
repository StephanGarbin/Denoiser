#include "BufferAggregator.h"

namespace Denoise
{

	BufferAggregator::BufferAggregator(const Dimension& dim, size_t numChannels)
	{

	}

	BufferAggregator::~BufferAggregator()
	{
		m_denominator.clear();
		m_denominator.clear();
	}

	void BufferAggregator::divideBuffers()
	{
		for (size_t c = 0; c < m_numerator.size(); ++c)
		{
			for (size_t i = 0; i < m_numerator[c].size(); ++i)
			{
				if (m_denominator[c][i] != 0.0f)
				{
					m_numerator[c][i] = m_numerator[c][i] / m_denominator[c][i];
				}
				else
				{
					m_numerator[c][i] = 0.0f;
				}
			}
		}
	}
}