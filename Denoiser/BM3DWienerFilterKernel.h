#pragma once

#include "BM3DSettings.h"
#include "common.h"
#include "TRANSFORM_DOMAIN_FORMATS.h"

#include <vector>
#include <fftw3.h>


namespace Denoise
{

	class BM3DWienerFilterKernel
	{
	public:
		BM3DWienerFilterKernel(const BM3DSettings& settings);
		~BM3DWienerFilterKernel();

		void processWienerFilter(DOMAIN_FORMAT* blockNoisy, DOMAIN_FORMAT* blockEstimate, index_t numPatches,
			index_t numChannels, std::vector<DOMAIN_FORMAT>& blockWeight, const std::vector<float>& stdDeviation);

		void processWienerFilterMeanAdaptive(DOMAIN_FORMAT* blockNoisy, DOMAIN_FORMAT* blockEstimate, index_t numPatches,
			index_t numChannels, std::vector<DOMAIN_FORMAT>& blockWeight, const std::vector<float>& stdDeviation);

	private:
		void initForwardTransforms();
		void initBackwardTransforms();

		void initNormalisationCoefficients();
		std::vector<DOMAIN_FORMAT> m_forwardCoefficients;
		std::vector<DOMAIN_FORMAT> m_backwardCoefficients;

		BM3DSettings m_settings;
		std::vector<index_t> m_transformLevels;

		//For WHT
		std::vector<DOMAIN_FORMAT> m_fwhtMem;

		//FFTW Stuff
		TRANSFORM_KIND* m_forwardTransformKind;
		PLAN_TYPE* m_forwardPlans;

		TRANSFORM_KIND* m_backwardTransformKind;
		PLAN_TYPE* m_backwardPlans;
	};

}

