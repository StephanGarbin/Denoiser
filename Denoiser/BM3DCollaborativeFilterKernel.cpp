#include "BM3DCollaborativeFilterKernel.h"

#include "Transforms.h"

namespace Denoise
{

	BM3DCollaborativeFilterKernel::BM3DCollaborativeFilterKernel(const BM3DSettings& settings)
	{
		m_settings = settings;

		//Initialise plans for powers of 2 (block depths)
		for (index_t level = 2; level < m_settings.numPatchesPerBlock; level*= 2)
		{
			m_transformLevels.push_back(level);
		}

		m_transformLevels.push_back(m_settings.numPatchesPerBlock);

		initForwardTransforms();
		initNormalisationCoefficients();

		m_fwhtMem.resize(128);
	}


	BM3DCollaborativeFilterKernel::~BM3DCollaborativeFilterKernel()
	{
		for (index_t i = 0; i < m_transformLevels.size(); ++i)
		{
			fftwf_destroy_plan(m_forwardPlans[m_transformLevels[i]]);
			fftwf_destroy_plan(m_backwardPlans[m_transformLevels[i]]);
		}

		delete[] m_forwardTransformKind;
		delete[] m_backwardTransformKind;

		m_forwardCoefficients.clear();
		m_backwardCoefficients.clear();
		m_fwhtMem.clear();
	}
	
	void BM3DCollaborativeFilterKernel::initForwardTransforms()
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

	void BM3DCollaborativeFilterKernel::initBackwardTransforms()
	{
		m_backwardPlans = new fftwf_plan[m_transformLevels.size()];

		float* in = new float[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlock];

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
	
	void BM3DCollaborativeFilterKernel::initNormalisationCoefficients()
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

	void BM3DCollaborativeFilterKernel::processCollaborativeFilter(float* block, index_t numPatches, float& blockWeight,
		float stdDeviation)
	{
		index_t totalSize = sqr(m_settings.patchSize) * numPatches;

		index_t planIdx = (index_t)(sqrtf((float)numPatches) - 1.0f);

		//DCT
		fftwf_execute_r2r(m_forwardPlans[planIdx], block, block);

		//Normalise DCT
		for (index_t patch = 0; patch < numPatches; ++patch)
		{
			for (index_t row = 0; row < m_settings.patchSize; ++row)
			{
				for (index_t col = 0; col < m_settings.patchSize; ++col)
				{
					block[patch * sqr(m_settings.patchSize) + row * m_settings.patchSize + col] *=
						m_forwardCoefficients[row * m_settings.patchSize + col];
				}
			}
		}

		//WHT
		for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
		{
			cfwht(block, 0, numPatches, numPatches, i, m_fwhtMem, sqr(m_settings.patchSize));
		}

		//Thresholding
		float numRetainedCoefficients = (float)(sqr(m_settings.patchSize) * numPatches);

		for (index_t patch = 0; patch < numPatches; ++patch)
		{
			for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
			{
				if (std::fabs(block[i + patch * sqr(m_settings.patchSize)]) <= stdDeviation * 2.7f * sqrtf((float)numPatches));
				{
					block[i + patch * sqr(m_settings.patchSize)] = 0.0f;
					numRetainedCoefficients -= 1.0f;
				}
			}
		}

		if (numRetainedCoefficients > 1.0f)
		{
			blockWeight = 1.0f / numRetainedCoefficients;
		}
		else
		{
			blockWeight = 1.0f;
		}

		//WHT
		for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
		{
			cfwht(block, 0, numPatches, numPatches, i, m_fwhtMem, sqr(m_settings.patchSize));
		}

		//Normalise DCT
		for (index_t patch = 0; patch < numPatches; ++patch)
		{
			for (index_t row = 0; row < m_settings.patchSize; ++row)
			{
				for (index_t col = 0; col < m_settings.patchSize; ++col)
				{
					block[patch * sqr(m_settings.patchSize) + row * m_settings.patchSize + col] *=
						m_backwardCoefficients[row * m_settings.patchSize + col];
				}
			}
		}

		//Inverse DCT
		fftwf_execute_r2r(m_backwardPlans[planIdx], block, block);
	}
}
