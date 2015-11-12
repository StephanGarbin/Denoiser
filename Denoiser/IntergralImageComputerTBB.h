#pragma once

#include <vector>
#include <utility>

#include "common.h"
#include "Image.h"
#include "ImageBlockProcessorSettings.h"
#include "ImageBlockProcessorSettingsInternal.h"

namespace Denoise
{

	class IntergralImageComputerTBB
	{
	public:
		IntergralImageComputerTBB(const ImageBlockProcessorSettings& settings,
			const ImageBlockProcessorSettingsInternal& settingsInternal,
			const std::vector<std::pair<int, int> >& shifts,
			std::vector<std::vector<std::vector<double> > >& distanceImage,
			std::vector<std::vector<std::vector<double> > >& integralImage,
			const Image& image);

		~IntergralImageComputerTBB();

		void operator()(const std::pair<size_t, size_t>& r) const;

	private:
		const std::vector<std::pair<int, int> >& m_shifts;
		ImageBlockProcessorSettings m_settings;
		ImageBlockProcessorSettingsInternal m_settingsInternal;

		std::vector<std::vector<std::vector<double> > >& m_distanceImage;
		std::vector<std::vector<std::vector<double> > >& m_integralImage;

		const Image& m_image;
	};

}
