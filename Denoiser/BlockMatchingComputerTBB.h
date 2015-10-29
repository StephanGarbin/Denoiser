#pragma once

#include <vector>
#include <utility>

#include <tbb\blocked_range.h>

#include "common.h"
#include "Image.h"
#include "Rectangle.h"
#include "SortedPatchCollection.h"
#include "ImageBlockProcessorSettings.h"
#include "ImageBlockProcessorSettingsInternal.h"

namespace Denoise
{

	class BlockMatchingComputerTBB
	{
	public:
		BlockMatchingComputerTBB(const ImageBlockProcessorSettings& settings,
			const ImageBlockProcessorSettingsInternal& settingsInternal,
			const std::vector<std::pair<int, int> >& shifts,
			std::vector<std::vector<std::vector<float> > >& distanceImage,
			std::vector<std::vector<std::vector<double> > >& integralImage,
			const Image& image,
			std::vector<SortedPatchCollection>& matchedBlocksSorted,
			const std::vector<Rectangle>& scanlines);

		~BlockMatchingComputerTBB();

		void operator()(const tbb::blocked_range<index_t>& r) const;

	private:
		const std::vector<std::pair<int, int> >& m_shifts;
		ImageBlockProcessorSettings m_settings;
		ImageBlockProcessorSettingsInternal m_settingsInternal;

		std::vector<std::vector<std::vector<float> > >& m_distanceImage;
		std::vector<std::vector<std::vector<double> > >& m_integralImage;

		const Image& m_image;

		std::vector<SortedPatchCollection>& m_matchedBlocksSorted;
		const std::vector<Rectangle>& m_scanlines;
	};

}
