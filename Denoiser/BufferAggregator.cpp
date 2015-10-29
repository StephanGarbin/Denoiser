#include "BufferAggregator.h"

#include <iostream>

namespace Denoise
{

	BufferAggregator::BufferAggregator(const Dimension& dim, index_t numChannels)
		: m_dim(dim), m_numChannels(numChannels)
	{
		m_numerator.resize(numChannels);
		m_denominator.resize(numChannels);

		for (index_t c = 0; c < m_numChannels; ++c)
		{
			m_numerator[c].resize(m_dim.height * m_dim.width);
			for (index_t i = 0; i < m_numerator[c].size(); ++i)
			{
				m_numerator[c][i] = 0.0f;
			}

			m_denominator[c].resize(m_dim.height * m_dim.width);
			for (index_t i = 0; i < m_denominator[c].size(); ++i)
			{
				m_denominator[c][i] = 0.0f;
			}
		}
	}

	BufferAggregator::~BufferAggregator()
	{
		m_denominator.clear();
		m_denominator.clear();
	}

	void BufferAggregator::divideBuffers()
	{
		for (index_t c = 0; c < m_numerator.size(); ++c)
		{
			for (index_t i = 0; i < m_numerator[c].size(); ++i)
			{
				if (m_denominator[c][i] != 0.0f)
				{
					m_numerator[c][i] = m_numerator[c][i] / m_denominator[c][i];
				}
				else
				{
					if (m_numerator[c][i] != 0)
					{
						std::cout << "Denominator 0!" << std::endl;
					}
					m_numerator[c][i] = 0.0f;
				}
			}
		}
	}

	void BufferAggregator::clear()
	{
		for (index_t c = 0; c < m_numChannels; ++c)
		{
			m_numerator[c].resize(m_dim.height * m_dim.width);
			for (index_t i = 0; i < m_numerator[c].size(); ++i)
			{
				m_numerator[c][i] = 0.0f;
			}

			m_denominator[c].resize(m_dim.height * m_dim.width);
			for (index_t i = 0; i < m_denominator[c].size(); ++i)
			{
				m_denominator[c][i] = 0.0f;
			}
		}
	}
}