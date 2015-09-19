#include <iostream>
#include <math.h>
#include <vector>

#include <fftw3.h>

void hardThreshold(double* block, double variance);
void dctTransform(double* block, double* result);
void normaliseDCT(double* block, bool inverse);
void inverseDctTransform(double* block, double* result);
void cfwht(double* x, int start, int n, int seqLength, int offset, std::vector<double>& fwhtMem, int stride);

/* The gateway function */
void bm3dDEBUG(double* block, double variance)
{
	std::vector<double> fwhtMem;
	fwhtMem.resize(128);

	int totalSize = 8 * 8 * 32;

	double* temp = new double[totalSize];

	normaliseDCT(block, false);
	dctTransform(block, temp);

	for (int i = 0; i < 64; ++i)
	{
		cfwht(block, 0, 32, 32, i, fwhtMem, 8 * 8);
	}

	hardThreshold(block, variance);

	for (int i = 0; i < 64; ++i)
	{
		cfwht(block, 0, 32, 32, i, fwhtMem, 8 * 8);
	}

	for (int i = 0; i < 32 * 8 * 8; ++i)
	{
		block[i] /= 32.0;
	}

	inverseDctTransform(block, block);
	normaliseDCT(block, true);

	for (int i = 0; i < totalSize; ++i)
	{
		block[i] /= (double)(8 * 2);
	}

	delete[] temp;
}

void hardThreshold(double* block, double variance)
{
	int totalSize = 8 * 8 * 32;

	for (int i = 0; i < totalSize; ++i)
	{
		if (std::abs(block[i]) <= variance * 2.7)
		{
			block[i] = 0.0;
		}
	}
}

void dctTransform(double* block, double* result)
{
	int patchSize = 8;
	int level = 32;

	double* in = new double[8 * 8 * 32];
	// double* out = new double[8 * 8 * 32];

	int n[2];
	n[0] = patchSize;
	n[1] = patchSize;
	int idist = n[0] * n[1];
	int odist = idist;
	int istride = 1;
	int ostride = 1;
	int *inembed = n;
	int *onembed = n;
	fftw_r2r_kind forwardKind[2];
	forwardKind[0] = FFTW_REDFT10;
	forwardKind[1] = FFTW_REDFT10;

	fftw_plan transformPlan = fftw_plan_many_r2r(2, n, 32, in, inembed, istride, idist,
		in, onembed, ostride, odist, forwardKind, FFTW_ESTIMATE);

	fftw_execute_r2r(transformPlan, block, block);

	fftw_destroy_plan(transformPlan);

	delete[] in;
	//delete[] out;
}


void inverseDctTransform(double* block, double* result)
{
	int patchSize = 8;
	int level = 32;

	double* in = new double[8 * 8 * 32];
	//double* out = new double[8 * 8 * 32];

	int n[2];
	n[0] = patchSize;
	n[1] = patchSize;
	int idist = n[0] * n[1];
	int odist = idist;
	int istride = 1;
	int ostride = 1;
	int *inembed = n;
	int *onembed = n;
	fftw_r2r_kind forwardKind[2];
	forwardKind[0] = FFTW_REDFT01;
	forwardKind[1] = FFTW_REDFT01;

	fftw_plan transformPlan = fftw_plan_many_r2r(2, n, 32, block, inembed, istride, idist,
		in, onembed, ostride, odist, forwardKind, FFTW_ESTIMATE);

	fftw_execute_r2r(transformPlan, block, block);

	fftw_destroy_plan(transformPlan);

	delete[] in;
	//delete[] out;
}

void normaliseDCT(double* block, bool inverse)
{
	int patchSize = 8;
	int numPatches = 32;

	double multFactor = 0.5 / (double)(patchSize);

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
						block[idx] *= 0.5 * multFactor;
					}
					else
					{
						block[idx] *= 2.0;
					}
				}
				else if (row * col == 0)
				{
					if (inverse)
					{
						block[idx] *= 0.7071067811865475 * multFactor;
					}
					else
					{
						block[idx] *= 1.414213562373095;
					}
				}
				else
				{
					if (inverse)
					{
						block[idx] *= 1.0 * multFactor;
					}
					else
					{
						block[idx] *= 1.0;
					}
				}
			}
		}
	}
}

void
cfwht(double* x, int start, int n, int seqLength, int offset, std::vector<double>& fwhtMem, int stride)
{
	if (n == 2)
	{
		double a = x[offset + start * stride];
		double b = x[offset + (start + 1) * stride];
		x[offset + start * stride] = a + b;
		x[offset + (start + 1) * stride] = a - b;
	}
	else
	{
		int halfN = n / 2;

		for (int i = 0; i < halfN; ++i)
		{
			double a = x[offset + (start + i * 2) * stride];
			double b = x[offset + (start + i * 2 + 1) * stride];
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
