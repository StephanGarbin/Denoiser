#include "IntergralImageComputerTBB.h"

#include "ImageBlockProcessorFunctions.h"

#include <iostream>

namespace Denoise
{

	IntergralImageComputerTBB::IntergralImageComputerTBB(const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal,
		const std::vector<std::pair<int, int> >& shifts,
		std::vector<std::vector<std::vector<double> > >& distanceImage,
		std::vector<std::vector<std::vector<double> > >& integralImage,
		const Image& image) :
		m_settings(settings), m_settingsInternal(settingsInternal), m_shifts(shifts),
		m_distanceImage(distanceImage), m_integralImage(integralImage),
		m_image(image)
	{

	}


	IntergralImageComputerTBB::~IntergralImageComputerTBB()
	{
	}

	void IntergralImageComputerTBB::operator()(const std::pair<size_t, size_t>& r) const
	{
		std::vector<std::pair<int, int> > localShifts;
		for (index_t i = r.first; i != r.second; ++i)
		{
			localShifts.push_back(m_shifts[i]);
		}

		computeIntegralImageForSpecificShiftsBlock(m_image,
			m_distanceImage, m_integralImage, localShifts,
			r.first, m_settings, m_settingsInternal);
	}

}
