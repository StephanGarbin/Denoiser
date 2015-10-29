#pragma once

#include <vector>

#include "common.h"
#include "Image.h"
#include "ImagePatch.h"
#include "Rectangle.h"
#include "IDX2.h"
#include "SortedPatchCollection.h"

#include "ImageBlockProcessorSettings.h"
#include "ImageBlockProcessorSettingsInternal.h"

#include <vector>
#include <utility>

namespace Denoise
{
	void computeBlockMatchingForSpecificShifts(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<float> >& distanceImage,
		std::vector<std::vector<double> >& integralImage,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal);

	void computeBlockMatchingForSpecificShifts_doNotComputeIntegralImage(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<float> >& distanceImage,
		std::vector<std::vector<double> >& integralImage,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal);

	void computeBlockMatchingForSpecificShifts_doNotComputeIntegralImageBlock(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<std::vector<float> > >& distanceImage,
		std::vector<std::vector<std::vector<double> > >& integralImage,
		const std::vector<std::pair<int, int> >& shifts,
		index_t startIdx,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal);

	void computeIntegralImageForSpecificShifts(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<float> >& distanceImage,
		std::vector<std::vector<double> >& integralImage,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal);

	void computeIntegralImageForSpecificShiftsBlock(const Image& image,
		std::vector<std::vector<std::vector<float> > >& distanceImage,
		std::vector<std::vector<std::vector<double> > >& integralImage,
		const std::vector<std::pair<int, int> >& shifts,
		index_t startIdx,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal);

	inline double patchDistanceIntegralImage(const std::vector<double>& integralImage, const ImagePatch& templatePatch,
		const Rectangle& imageBlock, const IDX2& position)
	{
		double result = integralImage[(position.row + templatePatch.height - 1) * imageBlock.width()
			+ position.col + templatePatch.width - 1];

		if (position.col > 0)
		{
			result -= integralImage[(position.row + templatePatch.height - 1) * imageBlock.width()
				+ position.col - 1];
		}

		if (position.row > 0)
		{
			result -= integralImage[(position.row - 1) * imageBlock.width()
				+ position.col + templatePatch.width - 1];
		}

		if (position.col * position.row > 0)
		{
			result += integralImage[(position.row - 1) * imageBlock.width()
				+ position.col - 1];
		}

		return result;
	}

	void computeIntegralImage(const std::vector<float>& pixels, const Rectangle& imageBlock,
		std::vector<double>& integralImage);
}
