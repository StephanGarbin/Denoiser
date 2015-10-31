#pragma once

#include "BM3DSettings.h"
#include "common.h"

#include <vector>
#include <fftw3.h>


namespace Denoise
{

	class BM3DCollaborativeFilterKernel
	{
	public:
		BM3DCollaborativeFilterKernel(const BM3DSettings& settings);
		~BM3DCollaborativeFilterKernel();

		void processCollaborativeFilter(float* block, index_t numPatches, index_t numChannels, std::vector<float>& blockWeight, float stdDeviation);

		void processCollaborativeFilterMeanAdaptive(float* block, index_t numPatches, index_t numChannels, std::vector<float>& blockWeight, float stdDeviation);


	private:
		void initForwardTransforms();
		void initBackwardTransforms();

		void initNormalisationCoefficients();
		std::vector<float> m_forwardCoefficients;
		std::vector<float> m_backwardCoefficients;

		BM3DSettings m_settings;
		std::vector<index_t> m_transformLevels;

		//For WHT
		std::vector<float> m_fwhtMem;

		//FFTW Stuff
		fftwf_r2r_kind* m_forwardTransformKind;
		fftwf_plan* m_forwardPlans;

		fftwf_r2r_kind* m_backwardTransformKind;
		fftwf_plan* m_backwardPlans;
	};

}

