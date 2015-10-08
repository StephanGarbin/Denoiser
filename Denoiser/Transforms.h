#pragma once

#include <vector>

namespace Denoise
{
	inline void cfwht(float* x, int start, int n, int seqLength, int offset, std::vector<float>& fwhtMem, int stride)
	{
		if (n == 1)
		{
			return;
		}
			if (n == 2)
			{
				float a = x[offset + start * stride];
				float b = x[offset + (start + 1) * stride];
				x[offset + start * stride] = a + b;
				x[offset + (start + 1) * stride] = a - b;
			}
			else
			{
				int halfN = n / 2;

				for (int i = 0; i < halfN; ++i)
				{
					float a = x[offset + (start + i * 2) * stride];
					float b = x[offset + (start + i * 2 + 1) * stride];
					x[offset + (start + i) * stride] = a + b;

					fwhtMem[start + i * 1] = a - b;
				}

				for (int i = start; i < start + halfN; ++i)
				{
					x[offset + (i + halfN) * stride] = fwhtMem[i];
				}

				cfwht(x, start + halfN, halfN, seqLength, offset, fwhtMem, stride);
				cfwht(x, start, halfN, seqLength, offset, fwhtMem, stride);
			}
	}


}