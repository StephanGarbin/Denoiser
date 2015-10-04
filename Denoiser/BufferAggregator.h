#pragma once

#include<vector>

#include "Dimension.h"
#include "common.h"

namespace Denoise
{
	class BufferAggregator
	{
	public:
		BufferAggregator(const Dimension& dim, index_t numChannels);
		~BufferAggregator();
	
		void divideBuffers();

		inline float getValueResult(index_t channel, index_t idx) { return m_numerator[channel][idx]; }
		inline float getValueResult(index_t channel, index_t row, index_t col) { return m_numerator[channel][IDX2_2_1(row, col)]; }

		inline float getValueNumerator(index_t channel, index_t idx) { return m_numerator[channel][idx]; }
		inline float getValueNumerator(index_t channel, index_t row, index_t col) { return m_numerator[channel][IDX2_2_1(row, col)]; }

		inline float getValueDenominator(index_t channel, index_t idx) { return m_denominator[channel][idx]; }
		inline float getValueDenominator(index_t channel, index_t row, index_t col) { return m_denominator[channel][IDX2_2_1(row, col)]; }

		inline void setValueResult(index_t channel, index_t idx, float value) { m_numerator[channel][idx] = value; }
		inline void setValueResult(index_t channel, index_t row, index_t col, float value) { m_numerator[channel][IDX2_2_1(row, col)]; }

		inline void setValueNumerator(index_t channel, index_t idx, float value) { m_numerator[channel][idx] = value; }
		inline void setValueNumerator(index_t channel, index_t row, index_t col, float value) { m_numerator[channel][IDX2_2_1(row, col)] = value; }

		inline void addValueNumerator(index_t channel, index_t idx, float value) { m_numerator[channel][idx] += value; }
		inline void addValueNumerator(index_t channel, index_t row, index_t col, float value) { m_numerator[channel][IDX2_2_1(row, col)] += value; }

		inline void setValueDenominator(index_t channel, index_t idx, float value) { m_denominator[channel][idx] = value; }
		inline void setValueDenominator(index_t channel, index_t row, index_t col, float value) { m_denominator[channel][IDX2_2_1(row, col)] = value; }

		inline void addValueDenominator(index_t channel, index_t idx, float value) { m_denominator[channel][idx] += value; }
		inline void addValueDenominator(index_t channel, index_t row, index_t col, float value) { m_denominator[channel][IDX2_2_1(row, col)] += value; }


		inline index_t IDX2_2_1(const index_t row, const index_t col);

	private:
		Dimension m_dim;
		index_t m_numChannels;
		std::vector<std::vector<float> > m_numerator;
		std::vector<std::vector<float> > m_denominator;
	};

	index_t BufferAggregator::IDX2_2_1(const index_t row, const index_t col)
	{
		return row * m_dim.width + col;
	}
}

