#include "BufferAggregator.h"

#include <iostream>

namespace Denoise
{

	BufferAggregator::BufferAggregator(const Dimension& dim, index_t numChannels, index_t numNumerators)
		: m_dim(dim), m_numChannels(numChannels)
	{
		//std::cout << numNumerators << " num Numerators" << std::endl;

		m_numerator.resize(numNumerators);
		for (index_t i = 0; i < m_numerator.size(); ++i)
		{
			m_numerator[i].resize(numChannels);
		}

		m_denominator.resize(numChannels);

		for (index_t c = 0; c < m_numChannels; ++c)
		{
			for (index_t n = 0; n < m_numerator.size(); ++n)
			{
				m_numerator[n][c].resize(m_dim.height * m_dim.width);
				for (index_t i = 0; i < m_numerator[n][c].size(); ++i)
				{
					m_numerator[n][c][i] = 0.0f;
				}
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
		//std::cout << "Dividing!" << std::endl;
		//Sum over numerators if any
		if (m_numerator.size() > 1)
		{
			for (index_t c = 0; c < m_numChannels; ++c)
			{
				for (index_t i = 0; i < m_numerator[0][c].size(); ++i)
				{
					float sum = 0.0f;
					for (index_t n = 0; n < m_numerator.size(); ++n)
					{
						sum += m_numerator[n][c][i];
					}

					m_numerator[0][c][i] = sum;
				}
			}
		}

		//Divide
		for (index_t c = 0; c < m_numerator[0].size(); ++c)
		{
			for (index_t i = 0; i < m_numerator[0][c].size(); ++i)
			{
				if (m_denominator[c][i] != 0.0f)
				{
					m_numerator[0][c][i] = m_numerator[0][c][i] / m_denominator[c][i];
				}
				else
				{
					if (m_numerator[0][c][i] != 0)
					{
						std::cout << "Denominator 0!" << std::endl;
					}
					m_numerator[0][c][i] = 0.0f;
				}
			}
		}
	}

	void BufferAggregator::clear()
	{
		for (index_t n = 0; n < m_numerator.size(); ++n)
		{
			for (index_t c = 0; c < m_numChannels; ++c)
			{
				m_numerator[n][c].resize(m_dim.height * m_dim.width);
				for (index_t i = 0; i < m_numerator[n][c].size(); ++i)
				{
					m_numerator[n][c][i] = 0.0f;
				}
			}
		}
		for (index_t c = 0; c < m_numChannels; ++c)
		{
			m_denominator[c].resize(m_dim.height * m_dim.width);
			for (index_t i = 0; i < m_denominator[c].size(); ++i)
			{
				m_denominator[c][i] = 0.0f;
			}
		}
	}
}
