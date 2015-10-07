#include "DEBUG_HELPER.h"

namespace Denoise
{

	/* The gateway function */
	void bm3dDEBUG(float* block, float stdDeviation, index_t patchSize, index_t numPatches,
		float& weight)
	{
		std::vector<float> fwhtMem;
		fwhtMem.resize(128);

		index_t totalSize = sqr(patchSize) * numPatches;

		float* temp = new float[totalSize];

		//normaliseDCT(block, false, patchSize, numPatches);
		dctTransform(block, temp, patchSize, numPatches);

		normaliseDCT_NEW(block, patchSize, numPatches, false);

		//for (index_t i = 0; i < sqr(patchSize); ++i)
		//{
		//	cfwht(block, 0, numPatches, numPatches, i, fwhtMem, sqr(patchSize));
		//}
		//hardThreshold(block, stdDeviation, patchSize, numPatches, weights);
		//for (int i = 0; i < sqr(patchSize); ++i)
		//{
		//	cfwht(block, 0, numPatches, numPatches, i, fwhtMem, sqr(patchSize));
		//}
		//for (int i = 0; i < totalSize; ++i)
		//{
		//	block[i] /= (float)numPatches;
		//}

		std::vector<float> blockVector(totalSize);
		for (index_t d = 0; d < numPatches; ++d)
		{
			for (index_t row = 0; row < patchSize; ++row)
			{
				for (index_t col = 0; col < patchSize; ++col)
				{
					index_t inputIdx = d * sqr(patchSize) + row * patchSize + col;
					index_t remappedIdx = (row * patchSize + col) * numPatches + d;
					blockVector[remappedIdx] = block[inputIdx];
				}
			}
		}
		std::vector<float> cache(100);
		std::vector<float> sigmaVector;
		sigmaVector.push_back(stdDeviation);
		std::vector<float> weights(3);
		ht_filtering_hadamard(blockVector, cache, numPatches, patchSize, 1, sigmaVector, 2.7f, weights, true);
		
		for (index_t d = 0; d < numPatches; ++d)
		{
			for (index_t row = 0; row < patchSize; ++row)
			{
				for (index_t col = 0; col < patchSize; ++col)
				{
					index_t inputIdx = d * sqr(patchSize) + row * patchSize + col;
					index_t remappedIdx = (row * patchSize + col) * numPatches + d;
					block[inputIdx] = blockVector[remappedIdx];
				}
			}
		}

		weight = weights[0];

		normaliseDCT_NEW(block, patchSize, numPatches, true);
		inverseDctTransform(block, block, patchSize, numPatches);
		//normaliseDCT(block, true, patchSize, numPatches);

		for (int i = 0; i < totalSize; ++i)
		{
			block[i] /= (float)(patchSize * 2);
		}

		delete[] temp;
	}

	void hardThreshold(float* block, float stdDeviation, index_t patchSize, index_t numPatches,
		std::vector<float>& weights)
	{
		float numRetainedCoefficients = (float)(sqr(patchSize) * numPatches);

		float factor = stdDeviation * 2.7f * sqrtf((float)numPatches);

		for (index_t patch = 0; patch < numPatches; ++patch)
		{
			for (index_t i = 0; i < sqr(patchSize); ++i)
			{
				//std::cout << stdDeviation * 2.7f * sqrtf((float)numPatches);
				if (std::fabs(block[i + patch * sqr(patchSize)]) <= factor)
				{
					//std::cout << std::fabs(block[i + patch * sqr(patchSize)]) << " < " << factor << ";";
					block[i + patch * sqr(patchSize)] = 0.0f;
					numRetainedCoefficients -= 1.0f;
				}
			}
		}

		if (numRetainedCoefficients > 1.0f)
		{
			weights[0] = 1.0f / numRetainedCoefficients;
		}
		else
		{
			weights[0] = 1.0f;
		}

		//std::cout << numRetainedCoefficients << "; ";
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

	/**
	* @brief HT filtering using Welsh-Hadamard transform (do only third
	*        dimension transform, Hard Thresholding and inverse transform).
	*
	* @param group_3D : contains the 3D block for a reference patch;
	* @param tmp: allocated vector used in Hadamard transform for convenience;
	* @param nSx_r : number of similar patches to a reference one;
	* @param kHW : size of patches (kHW x kHW);
	* @param chnls : number of channels of the image;
	* @param sigma_table : contains value of noise for each channel;
	* @param lambdaHard3D : value of thresholding;
	* @param weight_table: the weighting of this 3D group for each channel;
	* @param doWeight: if true process the weighting, do nothing
	*        otherwise.
	*
	* @return none.
	**/
	void ht_filtering_hadamard(
		std::vector<float> &group_3D
		, std::vector<float> &tmp
		, const unsigned nSx_r
		, const unsigned kHard
		, const unsigned chnls
		, std::vector<float> const& sigma_table
		, const float lambdaHard3D
		, std::vector<float> &weight_table
		, const bool doWeight
		){
		//! Declarations
		const unsigned kHard_2 = kHard * kHard;
		for (unsigned c = 0; c < chnls; c++)
			weight_table[c] = 0.0f;
		const float coef_norm = sqrtf((float)nSx_r);
		const float coef = 1.0f / (float)nSx_r;

		//! Process the Welsh-Hadamard transform on the 3rd dimension
		for (unsigned n = 0; n < kHard_2 * chnls; n++)
			hadamard_transform(group_3D, tmp, nSx_r, n * nSx_r);

		//! Hard Thresholding
		for (unsigned c = 0; c < chnls; c++)
		{
			const unsigned dc = c * nSx_r * kHard_2;
			const float T = lambdaHard3D * sigma_table[c] * coef_norm;
			//std::cout << "T = " << T << "; ST = " << sigma_table[c] << "; LH3d = " << lambdaHard3D << "; CN = " << coef_norm << ";; ";
			for (unsigned k = 0; k < kHard_2 * nSx_r; k++)
			{
				//std::cout << group_3D[k + dc] << "; ";
				if (fabs(group_3D[k + dc]) > T)
					weight_table[c]++;
				else
					group_3D[k + dc] = 0.0f;

				//std::cout << fabs(group_3D[k + dc]) << "; ";
			}
		}

		//! Process of the Welsh-Hadamard inverse transform
		for (unsigned n = 0; n < kHard_2 * chnls; n++)
			hadamard_transform(group_3D, tmp, nSx_r, n * nSx_r);

		for (unsigned k = 0; k < group_3D.size(); k++)
			group_3D[k] *= coef;

		//! Weight for aggregation
		if (doWeight)
		for (unsigned c = 0; c < chnls; c++)
			weight_table[c] = (weight_table[c] > 0.0f ? 1.0f / (float)
			(sigma_table[c] * sigma_table[c] * weight_table[c]) : 1.0f);
	}

	void hadamard_transform(
		std::vector<float> &vec
		, std::vector<float> &tmp
		, const unsigned N
		, const unsigned D
		){
		if (N == 1)
			return;
		else if (N == 2)
		{
			const float a = vec[D + 0];
			const float b = vec[D + 1];
			vec[D + 0] = a + b;
			vec[D + 1] = a - b;
		}
		else
		{
			const unsigned n = N / 2;
			for (unsigned k = 0; k < n; k++)
			{
				const float a = vec[D + 2 * k];
				const float b = vec[D + 2 * k + 1];
				vec[D + k] = a + b;
				tmp[k] = a - b;
			}
			for (unsigned k = 0; k < n; k++)
				vec[D + n + k] = tmp[k];

			hadamard_transform(vec, tmp, n, D);
			hadamard_transform(vec, tmp, n, D + n);
		}
	}

	void normaliseDCT_NEW(float* block, index_t patchSize, index_t numPatches, bool inverse)
	{
		std::vector<float> coef_norm(sqr(patchSize));
		std::vector<float> coef_norm_inv(sqr(patchSize));
		const float coef = 0.5f / ((float)(patchSize));
		for (unsigned i = 0; i < patchSize; i++)
		for (unsigned j = 0; j < patchSize; j++)
		{
			if (i == 0 && j == 0)
			{
				coef_norm[i * patchSize + j] = 0.5f * coef;
				coef_norm_inv[i * patchSize + j] = 2.0f;
			}
			else if (i * j == 0)
			{
				coef_norm[i * patchSize + j] = SQRT2_INV * coef;
				coef_norm_inv[i * patchSize + j] = SQRT2;
			}
			else
			{
				coef_norm[i * patchSize + j] = 1.0f * coef;
				coef_norm_inv[i * patchSize + j] = 1.0f;
			}
		}

		for (index_t d = 0; d < numPatches; ++d)
		{
			for (index_t row = 0; row < patchSize; ++row)
			{
				for (index_t col = 0; col < patchSize; ++col)
				{
					index_t inputIdx = d * sqr(patchSize) + row * patchSize + col;
					if (inverse)
					{
						block[inputIdx] *= coef_norm_inv[row * patchSize + col];
					}
					else
					{
						block[inputIdx] *= coef_norm[row * patchSize + col];
					}
				}
			}
		}
	}
}
