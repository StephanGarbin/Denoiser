#pragma once

#include<vector>

#include "Dimension.h"

namespace Denoise
{
	class BufferAggregator
	{
	public:
		BufferAggregator(const Dimension& dim, size_t numChannels);
		~BufferAggregator();
	
		void divideBuffers();

		inline float getValueResult(size_t channel, size_t idx) { return m_numerator[channel][idx]; }
		inline float getValueResult(size_t channel, size_t row, size_t col) { return m_numerator[channel][IDX2_2_1(row, col)]; }

		inline float getValueNumerator(size_t channel, size_t idx) { return m_numerator[channel][idx]; }
		inline float getValueNumerator(size_t channel, size_t row, size_t col) { return m_numerator[channel][IDX2_2_1(row, col)]; }

		inline float getValueDenominator(size_t channel, size_t idx) { return m_denominator[channel][idx]; }
		inline float getValueDenominator(size_t channel, size_t row, size_t col) { return m_denominator[channel][IDX2_2_1(row, col)]; }

		inline void setValueResult(size_t channel, size_t idx, float value) { m_numerator[channel][idx] = value; }
		inline void setValueResult(size_t channel, size_t row, size_t col, float value) { m_numerator[channel][IDX2_2_1(row, col)]; }

		inline void setValueNumerator(size_t channel, size_t idx, float value) { m_numerator[channel][idx] = value; }
		inline void setValueNumerator(size_t channel, size_t row, size_t col, float value) { m_numerator[channel][IDX2_2_1(row, col)] = value; }

		inline void addValueNumerator(size_t channel, size_t idx, float value) { m_numerator[channel][idx] += value; }
		inline void addValueNumerator(size_t channel, size_t row, size_t col, float value) { m_numerator[channel][IDX2_2_1(row, col)] += value; }

		inline void setValueDenominator(size_t channel, size_t idx, float value) { m_denominator[channel][idx] = value; }
		inline void setValueDenominator(size_t channel, size_t row, size_t col, float value) { m_denominator[channel][IDX2_2_1(row, col)] = value; }

		inline void addValueDenominator(size_t channel, size_t idx, float value) { m_denominator[channel][idx] += value; }
		inline void addValueDenominator(size_t channel, size_t row, size_t col, float value) { m_denominator[channel][IDX2_2_1(row, col)] += value; }


		inline size_t IDX2_2_1(const size_t row, const size_t col);

	private:
		Dimension m_dim;
		size_t m_numChannels;
		std::vector<std::vector<float> > m_numerator;
		std::vector<std::vector<float> > m_denominator;
	};

	size_t BufferAggregator::IDX2_2_1(const size_t row, const size_t col)
	{
		return row * m_dim.width + col;
	}
}

