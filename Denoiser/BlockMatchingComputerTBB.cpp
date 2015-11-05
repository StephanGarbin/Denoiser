#include "BlockMatchingComputerTBB.h"

#include "ImageBlockProcessorFunctions.h"

#include <iostream>

namespace Denoise
{

	BlockMatchingComputerTBB::BlockMatchingComputerTBB(const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal,
		const std::vector<std::pair<int, int> >& shifts,
		std::vector<std::vector<std::vector<double> > >& distanceImage,
		std::vector<std::vector<std::vector<double> > >& integralImage,
		const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		const std::vector<Rectangle>& scanlines) :
		m_settings(settings), m_settingsInternal(settingsInternal), m_shifts(shifts),
		m_distanceImage(distanceImage), m_integralImage(integralImage),
		m_image(image), m_matchedBlocksSorted(matchedBlocksSorted),
		m_scanlines(scanlines)
	{

	}


	BlockMatchingComputerTBB::~BlockMatchingComputerTBB()
	{
	}


	void BlockMatchingComputerTBB::operator()(const tbb::blocked_range<index_t>& r) const
	{
		for (index_t i = r.begin(); i != r.end(); ++i)
		{
			ImageBlockProcessorSettings localSettings = m_settings;
			localSettings.imageBlock = m_scanlines[i];

			ImageBlockProcessorSettingsInternal localInternalSettings = m_settingsInternal;

			if (i == m_scanlines.size() - 1)
			{
				localInternalSettings.iterateAtBorders = true;
			}
			else
			{
				localInternalSettings.iterateAtBorders = false;
			}
			
			localInternalSettings.offsetCols = 0; localInternalSettings.offsetRows = 0;

			computeBlockMatchingForSpecificShifts_doNotComputeIntegralImageBlock(m_image,
				m_matchedBlocksSorted, m_distanceImage, m_integralImage,
				m_shifts, 0, localSettings, localInternalSettings);
		}
	}

}