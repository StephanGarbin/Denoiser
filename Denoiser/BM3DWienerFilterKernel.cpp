#include "BM3DWienerFilterKernel.h"

#include "Transforms.h"
#include <iostream>

namespace Denoise
{

	BM3DWienerFilterKernel::BM3DWienerFilterKernel(const BM3DSettings& settings)
	{
		m_settings = settings;

		m_transformLevels.push_back(1);

		//Initialise plans for powers of 2 (block depths)
		for (index_t level = 2; level < m_settings.numPatchesPerBlockWiener; level *= 2)
		{
			m_transformLevels.push_back(level);
		}

		m_transformLevels.push_back(m_settings.numPatchesPerBlockWiener);

		initForwardTransforms();
		initBackwardTransforms();
		initNormalisationCoefficients();

		m_fwhtMem.resize(128);
	}

	BM3DWienerFilterKernel::~BM3DWienerFilterKernel()
	{
		for (index_t i = 0; i < m_transformLevels.size(); ++i)
		{
			fftwf_destroy_plan(m_forwardPlans[i]);
			fftwf_destroy_plan(m_backwardPlans[i]);
		}

		delete[] m_forwardTransformKind;
		delete[] m_backwardTransformKind;

		m_forwardCoefficients.clear();
		m_backwardCoefficients.clear();
		m_fwhtMem.clear();
	}

	void BM3DWienerFilterKernel::initForwardTransforms()
	{
		m_forwardPlans = new fftwf_plan[m_transformLevels.size()];

		int n[2];
		n[0] = m_settings.patchSize;
		n[1] = m_settings.patchSize;
		int idist = n[0] * n[1];
		int odist = idist;
		int istride = 1;
		int ostride = 1;
		int *inembed = n;
		int *onembed = n;

		m_forwardTransformKind = new fftwf_r2r_kind[2];
		m_forwardTransformKind[0] = FFTW_REDFT10;
		m_forwardTransformKind[1] = FFTW_REDFT10;

		for (index_t i = 0; i < m_transformLevels.size(); ++i)
		{
			float* in = new float[sqr(m_settings.patchSize) * m_transformLevels[i]];
			float* out = new float[sqr(m_settings.patchSize) * m_transformLevels[i]];

			m_forwardPlans[i] = fftwf_plan_many_r2r(2, n, m_transformLevels[i], in, inembed, istride, idist,
				out, onembed, ostride, odist, m_forwardTransformKind, FFTW_ESTIMATE);

			delete[] in;
			delete[] out;
		}
	}

	void BM3DWienerFilterKernel::initBackwardTransforms()
	{
		m_backwardPlans = new fftwf_plan[m_transformLevels.size()];

		float* in = new float[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlockWiener];

		int n[2];
		n[0] = m_settings.patchSize;
		n[1] = m_settings.patchSize;
		int idist = n[0] * n[1];
		int odist = idist;
		int istride = 1;
		int ostride = 1;
		int *inembed = n;
		int *onembed = n;

		m_backwardTransformKind = new fftwf_r2r_kind[2];
		m_backwardTransformKind[0] = FFTW_REDFT01;
		m_backwardTransformKind[1] = FFTW_REDFT01;

		for (index_t i = 0; i < m_transformLevels.size(); ++i)
		{
			float* in = new float[sqr(m_settings.patchSize) * m_transformLevels[i]];
			float* out = new float[sqr(m_settings.patchSize) * m_transformLevels[i]];

			m_backwardPlans[i] = fftwf_plan_many_r2r(2, n, m_transformLevels[i], out, inembed, istride, idist,
				in, onembed, ostride, odist, m_backwardTransformKind, FFTW_ESTIMATE);

			delete[] in;
			delete[] out;
		}

		delete[] in;
	}

	void BM3DWienerFilterKernel::initNormalisationCoefficients()
	{
		m_forwardCoefficients.resize(sqr(m_settings.patchSize));
		m_backwardCoefficients.resize(sqr(m_settings.patchSize));

		//This Normalisation is based on Matlab's implementation
		for (index_t row = 0; row < m_settings.patchSize; ++row)
		{
			for (index_t col = 0; col < m_settings.patchSize; ++col)
			{
				index_t idx = row * m_settings.patchSize + col;

				if (row == 0 && col == 0)
				{
					m_forwardCoefficients[idx] = 0.5f * (0.5f / (float)(m_settings.patchSize));
					m_backwardCoefficients[idx] = 2.0f;
				}
				else if (row * col == 0)
				{
					m_forwardCoefficients[idx] = (1.0f / sqrtf(2.0f)) * (0.5f / (float)(m_settings.patchSize));
					m_backwardCoefficients[idx] = sqrtf(2.0f);
				}
				else
				{
					m_forwardCoefficients[idx] = 1.0f * (0.5f / (float)(m_settings.patchSize));
					m_backwardCoefficients[idx] = 1.0f;
				}
			}
		}
	}

	void BM3DWienerFilterKernel::processWienerFilter(float* blockNoisy, float* blockEstimate, index_t numPatches, index_t numChannels, std::vector<float>& blockWeight,
		float stdDeviation)
	{
		index_t totalSize = sqr(m_settings.patchSize) * numPatches;

		index_t planIdx;
		switch (numPatches)
		{
		case 1:
			planIdx = 0;
			break;
		case 2:
			planIdx = 1;
			break;
		case 4:
			planIdx = 2;
			break;
		case 8:
			planIdx = 3;
			break;
		case 16:
			planIdx = 4;
			break;
		case 32:
			planIdx = 5;
			break;
		default:
			std::cout << "ERROR: No DCT plan available for numBlocks = " << numPatches << std::endl;
			return;
			break;
		}

		for (index_t c = 0; c < numChannels; ++c)
		{
			//reset weight
			blockWeight[c] = 0.0f;

			index_t colourOffset = c * totalSize;

			//DCT for ESTIMATE
			fftwf_execute_r2r(m_forwardPlans[planIdx], blockEstimate + colourOffset, blockEstimate + colourOffset);

			//Normalise DCT for ESTIMATE
			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t row = 0; row < m_settings.patchSize; ++row)
				{
					for (index_t col = 0; col < m_settings.patchSize; ++col)
					{
						blockEstimate[colourOffset + patch * sqr(m_settings.patchSize) + row * m_settings.patchSize + col] *=
							m_forwardCoefficients[row * m_settings.patchSize + col];
					}
				}
			}

			//WHT for ESTIMATE
			for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
			{
				cfwht(blockEstimate, 0, numPatches, numPatches, colourOffset + i, m_fwhtMem, sqr(m_settings.patchSize));
			}

			//DCT for NOISY
			fftwf_execute_r2r(m_forwardPlans[planIdx], blockNoisy + colourOffset, blockNoisy + colourOffset);

			//Normalise DCT for NOISY
			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t row = 0; row < m_settings.patchSize; ++row)
				{
					for (index_t col = 0; col < m_settings.patchSize; ++col)
					{
						blockNoisy[colourOffset + patch * sqr(m_settings.patchSize) + row * m_settings.patchSize + col] *=
							m_forwardCoefficients[row * m_settings.patchSize + col];
					}
				}
			}

			//WHT for NOISY
			for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
			{
				cfwht(blockNoisy, 0, numPatches, numPatches, colourOffset + i, m_fwhtMem, sqr(m_settings.patchSize));
			}

			//Apply Wiener Filter
			float factor =1.0f / (float)numPatches;

			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
				{
					float coeff = factor * sqr(blockEstimate[colourOffset + patch * sqr(m_settings.patchSize) + i]);
					float mult = coeff / (sqr(m_settings.stdDeviation) + coeff);

					blockNoisy[colourOffset + patch * sqr(m_settings.patchSize) + i] =
						blockNoisy[colourOffset + patch * sqr(m_settings.patchSize) + i] * mult * factor;

					blockWeight[c] += mult;
				}
			}

			//WHT for NOISY
			for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
			{
				cfwht(blockNoisy, 0, numPatches, numPatches, colourOffset + i, m_fwhtMem, sqr(m_settings.patchSize));
			}

			//Normalise WHT for NOISY
			//for (index_t i = 0; i < totalSize; ++i)
			//{
			//	blockNoisy[colourOffset + i] /= (float)numPatches;
			//}

			//Normalise DCT for NOISY
			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t row = 0; row < m_settings.patchSize; ++row)
				{
					for (index_t col = 0; col < m_settings.patchSize; ++col)
					{
						blockNoisy[colourOffset + patch * sqr(m_settings.patchSize) + row * m_settings.patchSize + col] *=
							m_backwardCoefficients[row * m_settings.patchSize + col];
					}
				}
			}

			//Inverse DCT for NOISY
			fftwf_execute_r2r(m_backwardPlans[planIdx], blockNoisy + colourOffset, blockNoisy + colourOffset);

			//normalise again
			for (index_t i = 0; i < totalSize; ++i)
			{
				blockNoisy[colourOffset + i] /= (float)(m_settings.patchSize * 2);
			}

			if (blockWeight[c] > 1.0f)
			{
				blockWeight[c] = 1.0f / (std::pow(m_settings.stdDeviation, 2) * blockWeight[c]);
			}
			else
			{
				blockWeight[c] = 1.0f;
			}
		}
	}
}

