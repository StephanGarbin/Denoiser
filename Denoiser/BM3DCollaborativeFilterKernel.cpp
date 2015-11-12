#include "BM3DCollaborativeFilterKernel.h"

#include "Transforms.h"
#include "Statistics.h"
#include <iostream>

namespace Denoise
{

	BM3DCollaborativeFilterKernel::BM3DCollaborativeFilterKernel(const BM3DSettings& settings)
	{
		m_settings = settings;

		m_transformLevels.push_back(1);

		//Initialise plans for powers of 2 (block depths)
		for (index_t level = 2; level < m_settings.numPatchesPerBlockCollaborative; level*= 2)
		{
			m_transformLevels.push_back(level);
		}

		m_transformLevels.push_back(m_settings.numPatchesPerBlockCollaborative);

		initForwardTransforms();
		initBackwardTransforms();
		initNormalisationCoefficients();

		m_fwhtMem.resize(128);
	}


	BM3DCollaborativeFilterKernel::~BM3DCollaborativeFilterKernel()
	{
		for (index_t i = 0; i < m_transformLevels.size(); ++i)
		{
			PLAN_DTOR(m_forwardPlans[i]);
			PLAN_DTOR(m_backwardPlans[i]);
		}

		delete[] m_forwardTransformKind;
		delete[] m_backwardTransformKind;

		m_forwardCoefficients.clear();
		m_backwardCoefficients.clear();
		m_fwhtMem.clear();
	}
	
	void BM3DCollaborativeFilterKernel::initForwardTransforms()
	{
		m_forwardPlans = new PLAN_TYPE[m_transformLevels.size()];

		int n[2];
		n[0] = m_settings.patchSize;
		n[1] = m_settings.patchSize;
		int idist = n[0] * n[1];
		int odist = idist;
		int istride = 1;
		int ostride = 1;
		int *inembed = n;
		int *onembed = n;

		m_forwardTransformKind = new TRANSFORM_KIND[2];
		m_forwardTransformKind[0] = FFTW_REDFT10;
		m_forwardTransformKind[1] = FFTW_REDFT10;

		for (index_t i = 0; i < m_transformLevels.size(); ++i)
		{
			DOMAIN_FORMAT* in = new DOMAIN_FORMAT[sqr(m_settings.patchSize) * m_transformLevels[i]];
			DOMAIN_FORMAT* out = new DOMAIN_FORMAT[sqr(m_settings.patchSize) * m_transformLevels[i]];

			m_forwardPlans[i] = PLAN_CTOR(2, n, m_transformLevels[i], in, inembed, istride, idist,
				out, onembed, ostride, odist, m_forwardTransformKind, FFTW_ESTIMATE);

			delete[] in;
			delete[] out;
		}
	}

	void BM3DCollaborativeFilterKernel::initBackwardTransforms()
	{
		m_backwardPlans = new PLAN_TYPE[m_transformLevels.size()];

		DOMAIN_FORMAT* in = new DOMAIN_FORMAT[sqr(m_settings.patchSize) * m_settings.numPatchesPerBlockCollaborative];

		int n[2];
		n[0] = m_settings.patchSize;
		n[1] = m_settings.patchSize;
		int idist = n[0] * n[1];
		int odist = idist;
		int istride = 1;
		int ostride = 1;
		int *inembed = n;
		int *onembed = n;

		m_backwardTransformKind = new TRANSFORM_KIND[2];
		m_backwardTransformKind[0] = FFTW_REDFT01;
		m_backwardTransformKind[1] = FFTW_REDFT01;

		for (index_t i = 0; i < m_transformLevels.size(); ++i)
		{
			DOMAIN_FORMAT* in = new DOMAIN_FORMAT[sqr(m_settings.patchSize) * m_transformLevels[i]];
			DOMAIN_FORMAT* out = new DOMAIN_FORMAT[sqr(m_settings.patchSize) * m_transformLevels[i]];

			m_backwardPlans[i] = PLAN_CTOR(2, n, m_transformLevels[i], out, inembed, istride, idist,
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
					m_forwardCoefficients[idx] = 0.5f * (0.5f / (DOMAIN_FORMAT)(m_settings.patchSize));
					m_backwardCoefficients[idx] = 2.0f;
				}
				else if (row * col == 0)
				{
					m_forwardCoefficients[idx] = (1.0f / sqrtf(2.0f)) * (0.5f / (DOMAIN_FORMAT)(m_settings.patchSize));
					m_backwardCoefficients[idx] = sqrtf(2.0f);
				}
				else
				{
					m_forwardCoefficients[idx] = 1.0f * (0.5f / (DOMAIN_FORMAT)(m_settings.patchSize));
					m_backwardCoefficients[idx] = 1.0f;
				}
			}
		}
	}

	void BM3DCollaborativeFilterKernel::processCollaborativeFilter(DOMAIN_FORMAT* block, index_t numPatches, index_t numChannels,
		std::vector<DOMAIN_FORMAT>& blockWeight,
		const std::vector<DOMAIN_FORMAT>& stdDeviation)
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

		std::vector<DOMAIN_FORMAT> numRetainedCoefficients(numChannels);
		for (index_t i = 0; i < numRetainedCoefficients.size(); ++i)
		{
			numRetainedCoefficients[i] = (DOMAIN_FORMAT)(totalSize);
		}

		for (index_t c = 0; c < numChannels; ++c)
		{
			index_t colourOffset = c * totalSize;
			//DCT
			PLAN_EXECUTOR(m_forwardPlans[planIdx], block + colourOffset, block + colourOffset);

			//Normalise DCT
			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t row = 0; row < m_settings.patchSize; ++row)
				{
					for (index_t col = 0; col < m_settings.patchSize; ++col)
					{
						block[colourOffset + patch * sqr(m_settings.patchSize) + row * m_settings.patchSize + col] *=
							m_forwardCoefficients[row * m_settings.patchSize + col];
					}
				}
			}

			//WHT
			for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
			{
				cfwht(block, 0, numPatches, numPatches, colourOffset + i, m_fwhtMem, sqr(m_settings.patchSize));
			}

			//Thresholding
			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
				{
					if (std::fabs(block[colourOffset + i + patch * sqr(m_settings.patchSize)]) <= stdDeviation[c] * 2.7f * sqrtf((DOMAIN_FORMAT)numPatches))
					{
						block[colourOffset + i + patch * sqr(m_settings.patchSize)] = 0.0f;
						numRetainedCoefficients[c] -= 1.0f;
					}
				}
			}

			//WHT
			for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
			{
				cfwht(block, 0, numPatches, numPatches, colourOffset + i, m_fwhtMem, sqr(m_settings.patchSize));
			}

			//Normalise WHT
			for (index_t i = 0; i < totalSize; ++i)
			{
				block[colourOffset + i] /= (DOMAIN_FORMAT)numPatches;
			}

			//Normalise DCT
			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t row = 0; row < m_settings.patchSize; ++row)
				{
					for (index_t col = 0; col < m_settings.patchSize; ++col)
					{
						block[colourOffset + patch * sqr(m_settings.patchSize) + row * m_settings.patchSize + col] *=
							m_backwardCoefficients[row * m_settings.patchSize + col];
					}
				}
			}

			//Inverse DCT
			PLAN_EXECUTOR(m_backwardPlans[planIdx], block + colourOffset, block + colourOffset);

			//normalise again
			for (index_t i = 0; i < totalSize; ++i)
			{
				block[colourOffset + i] /= (DOMAIN_FORMAT)(m_settings.patchSize * 2);
			}

			if (numRetainedCoefficients[c] > 1.0f)
			{
				blockWeight[c] = 1.0f / (std::pow(m_settings.stdDeviation[c], 2) * numRetainedCoefficients[c]);
			}
			else
			{
				blockWeight[c] = 1.0f;
			}
		}
	}

	void BM3DCollaborativeFilterKernel::processCollaborativeFilterMeanAdaptive(DOMAIN_FORMAT* block, index_t numPatches, index_t numChannels,
		std::vector<DOMAIN_FORMAT>& blockWeight,
		const std::vector<DOMAIN_FORMAT>& stdDeviation)
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

		//Compute Mean for Adaptive Measure ----------------
		std::vector<DOMAIN_FORMAT> patchMeans(numChannels * sqr(m_settings.patchSize));
		calculateBlockMeans(block, numPatches, m_settings.patchSize, numChannels, &patchMeans[0]);
		//--------------------------------------------------

		std::vector<DOMAIN_FORMAT> numRetainedCoefficients(numChannels);
		for (index_t i = 0; i < numRetainedCoefficients.size(); ++i)
		{
			numRetainedCoefficients[i] = (DOMAIN_FORMAT)(totalSize);
		}

		for (index_t c = 0; c < numChannels; ++c)
		{
			index_t colourOffset = c * totalSize;
			//DCT
			PLAN_EXECUTOR(m_forwardPlans[planIdx], block + colourOffset, block + colourOffset);

			//Normalise DCT
			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t row = 0; row < m_settings.patchSize; ++row)
				{
					for (index_t col = 0; col < m_settings.patchSize; ++col)
					{
						block[colourOffset + patch * sqr(m_settings.patchSize) + row * m_settings.patchSize + col] *=
							m_forwardCoefficients[row * m_settings.patchSize + col];
					}
				}
			}

			//WHT
			for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
			{
				cfwht(block, 0, numPatches, numPatches, colourOffset + i, m_fwhtMem, sqr(m_settings.patchSize));
			}

			//Thresholding
			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
				{
					DOMAIN_FORMAT adaptiveFactor = calculateMeanAdaptiveFactor(stdDeviation[c], patchMeans[c * sqr(m_settings.patchSize) + i],
						m_settings.meanAdaptiveThresholdingFactor, m_settings.meanAdaptiveThresholdingPower);

					if (std::fabs(block[colourOffset + i + patch * sqr(m_settings.patchSize)]) <= stdDeviation[c] * 2.7f * sqrtf((DOMAIN_FORMAT)numPatches) * adaptiveFactor)
					{
						block[colourOffset + i + patch * sqr(m_settings.patchSize)] = 0.0f;
						numRetainedCoefficients[c] -= 1.0f;
					}
				}
			}

			//WHT
			for (index_t i = 0; i < sqr(m_settings.patchSize); ++i)
			{
				cfwht(block, 0, numPatches, numPatches, colourOffset + i, m_fwhtMem, sqr(m_settings.patchSize));
			}

			//Normalise WHT
			for (index_t i = 0; i < totalSize; ++i)
			{
				block[colourOffset + i] /= (DOMAIN_FORMAT)numPatches;
			}

			//Normalise DCT
			for (index_t patch = 0; patch < numPatches; ++patch)
			{
				for (index_t row = 0; row < m_settings.patchSize; ++row)
				{
					for (index_t col = 0; col < m_settings.patchSize; ++col)
					{
						block[colourOffset + patch * sqr(m_settings.patchSize) + row * m_settings.patchSize + col] *=
							m_backwardCoefficients[row * m_settings.patchSize + col];
					}
				}
			}

			//Inverse DCT
			PLAN_EXECUTOR(m_backwardPlans[planIdx], block + colourOffset, block + colourOffset);

			//normalise again
			for (index_t i = 0; i < totalSize; ++i)
			{
				block[colourOffset + i] /= (DOMAIN_FORMAT)(m_settings.patchSize * 2);
			}

			if (numRetainedCoefficients[c] > 1.0f)
			{
				blockWeight[c] = 1.0f / (std::pow(m_settings.stdDeviation[c], 2) * numRetainedCoefficients[c]);
			}
			else
			{
				blockWeight[c] = 1.0f;
			}
		}
	}
}
