#include <iostream>
#include <math.h>
#include <vector>

#include <fftw3.h>

#include "common.h"

namespace Denoise
{

	void hardThreshold(float* block, float stdDeviation, index_t patchSize, index_t numPatches,
		std::vector<float>& weights);

	void dctTransform(float* block, float* result, index_t patchSize, index_t numPatches);
	void normaliseDCT(float* block, bool inverse, index_t patchSize, index_t numPatches);
	void inverseDctTransform(float* block, float* result, index_t patchSize, index_t numPatches);
	void cfwht(float* x, int start, int n, int seqLength, int offset, std::vector<float>& fwhtMem, int stride);

	/* The gateway function */
	void bm3dDEBUG(float* block, float stdDeviation, index_t patchSize, index_t numPatches,
		std::vector<float>& weights)
	{
		std::vector<float> fwhtMem;
		fwhtMem.resize(128);

		index_t totalSize = sqr(patchSize) * numPatches;

		float* temp = new float[totalSize];

		normaliseDCT(block, false, patchSize, numPatches);
		dctTransform(block, temp, patchSize, numPatches);

		for (index_t i = 0; i < sqr(patchSize); ++i)
		{
			cfwht(block, 0, numPatches, numPatches, i, fwhtMem, sqr(patchSize));
		}

		hardThreshold(block, stdDeviation, patchSize, numPatches, weights);

		for (int i = 0; i < sqr(patchSize); ++i)
		{
			cfwht(block, 0, numPatches, numPatches, i, fwhtMem, sqr(patchSize));
		}

		for (int i = 0; i < totalSize; ++i)
		{
			block[i] /= (float)numPatches;
		}

		inverseDctTransform(block, block, patchSize, numPatches);
		normaliseDCT(block, true, patchSize, numPatches);

		for (int i = 0; i < totalSize; ++i)
		{
			block[i] /= (float)(patchSize * 2);
		}

		delete[] temp;
	}

	void hardThreshold(float* block, float stdDeviation, index_t patchSize, index_t numPatches,
		std::vector<float>& weights)
	{
		for (index_t patch = 0; patch < numPatches; ++patch)
		{
			float numRetainedCoefficients = (float)sqr(patchSize);

			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				if (std::abs(block[i + patch * sqr(patchSize)]) <= stdDeviation * 2.7f)
				{
					block[i + patch * sqr(patchSize)] = 0.0f;
					numRetainedCoefficients -= 1.0f;
				}
			}
			if (numRetainedCoefficients > 1.0f)
			{
				weights[patch] = 1.0f / numRetainedCoefficients;
			}
			else
			{
				weights[patch] = 1.0f;
			}
		}

	}

	void dctTransform(float* block, float* result, index_t patchSize, index_t numPatches)
	{
		int level = numPatches;

		float* in = new float[sqr(patchSize) * numPatches];

		int n[2];
		n[0] = patchSize;
		n[1] = patchSize;
		int idist = n[0] * n[1];
		int odist = idist;
		int istride = 1;
		int ostride = 1;
		int *inembed = n;
		int *onembed = n;
		fftwf_r2r_kind forwardKind[2];
		forwardKind[0] = FFTW_REDFT10;
		forwardKind[1] = FFTW_REDFT10;

		fftwf_plan transformPlan = fftwf_plan_many_r2r(2, n, numPatches, in, inembed, istride, idist,
			in, onembed, ostride, odist, forwardKind, FFTW_ESTIMATE);

		fftwf_execute_r2r(transformPlan, block, block);

		fftwf_destroy_plan(transformPlan);

		delete[] in;
	}


	void inverseDctTransform(float* block, float* result, index_t patchSize, index_t numPatches)
	{
		int level = numPatches;

		float* in = new float[sqr(patchSize) * numPatches];

		int n[2];
		n[0] = patchSize;
		n[1] = patchSize;
		int idist = n[0] * n[1];
		int odist = idist;
		int istride = 1;
		int ostride = 1;
		int *inembed = n;
		int *onembed = n;
		fftwf_r2r_kind forwardKind[2];
		forwardKind[0] = FFTW_REDFT01;
		forwardKind[1] = FFTW_REDFT01;

		fftwf_plan transformPlan = fftwf_plan_many_r2r(2, n, numPatches, block, inembed, istride, idist,
			in, onembed, ostride, odist, forwardKind, FFTW_ESTIMATE);

		fftwf_execute_r2r(transformPlan, block, block);

		fftwf_destroy_plan(transformPlan);

		delete[] in;
	}

	void normaliseDCT(float* block, bool inverse, index_t patchSize, index_t numPatches)
	{

		float multFactor = 0.5f / (float)(patchSize);

		for (int p = 0; p < numPatches; ++p)
		{
			int blockIdx = p * patchSize * patchSize;
			for (int row = 0; row < patchSize; ++row)
			{
				for (int col = 0; col < patchSize; ++col)
				{
					int idx = blockIdx + row * patchSize + col;

					if (row == 0 && col == 0)
					{
						if (inverse)
						{
							block[idx] *= 0.5f * multFactor;
						}
						else
						{
							block[idx] *= 2.0f;
						}
					}
					else if (row * col == 0)
					{
						if (inverse)
						{
							block[idx] *= 0.7071067811865475f * multFactor;
						}
						else
						{
							block[idx] *= 1.414213562373095f;
						}
					}
					else
					{
						if (inverse)
						{
							block[idx] *= 1.0f * multFactor;
						}
						else
						{
							block[idx] *= 1.0f;
						}
					}
				}
			}
		}
	}

	void
		cfwht(float* x, int start, int n, int seqLength, int offset, std::vector<float>& fwhtMem, int stride)
	{
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
