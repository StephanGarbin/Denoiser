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
#include <iostream>
#include <numeric>
#include <algorithm>

namespace Denoise
{
	void printBuffer(const std::vector<double>& pixels, int height, int width, const std::string& fileName);

	void computeBlockMatchingForSpecificShifts(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<double> >& distanceImage,
		std::vector<std::vector<double> >& integralImage,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal);

	void computeBlockMatchingForSpecificShifts_doNotComputeIntegralImage(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<double> >& distanceImage,
		std::vector<std::vector<double> >& integralImage,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal);

	void computeBlockMatchingForSpecificShifts_doNotComputeIntegralImageBlock(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<std::vector<double> > >& distanceImage,
		std::vector<std::vector<std::vector<double> > >& integralImage,
		const std::vector<std::pair<int, int> >& shifts,
		index_t startIdx,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal);

	void computeIntegralImageForSpecificShifts(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<double> >& distanceImage,
		std::vector<std::vector<double> >& integralImage,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal);

	void computeIntegralImageForSpecificShiftsBlock(const Image& image,
		std::vector<std::vector<std::vector<double> > >& distanceImage,
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

	void computeIntegralImage(const std::vector<double>& pixels, const Rectangle& imageBlock,
		std::vector<double>& integralImage);

	inline double computeDistanceForShift(std::vector<std::vector<double> >& integralImage,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal,
		int row, int col)
	{
		double distance = 0.0;
		for (index_t c = 0; c < settings.numChannelsToUse; ++c)
		{
			distance += patchDistanceIntegralImage(integralImage[c],
				settings.templatePatch, settingsInternal.accessibleImageBlock,
				IDX2(row - settings.imageBlock.bottom + settingsInternal.offsetRows, col - settings.imageBlock.left + settingsInternal.offsetCols));
		}

		distance = std::abs(distance);

		distance /= (double)settings.numChannelsToUse;

		if (settingsInternal.shiftCols == 0 && settingsInternal.shiftRows == 0)
		{
			distance = 0.0;
		}

		//std::cout << "Computed ;;";
		return distance;
	}

	inline bool checkRow(const Image& image,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal,
		int row)
	{
		if (row + (int)settingsInternal.offsetRows + settingsInternal.shiftRows < 0)
		{
			//std::cout << "Refused (low): " << row + settingsInternal.shiftRows << "; ";
			return false;
		}

		if (std::max<int>(row + settingsInternal.shiftRows, row) > image.height() - settings.templatePatch.height)
		{
			//std::cout << "Refused (high): " << row + settingsInternal.shiftRows  << "; ";
			return false;
		}

		//std::cout << row + settingsInternal.shiftRows << ", ";
		return true;
	}

	inline bool checkCol(const Image& image, 
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal,
		int col)
	{
		if (col + settingsInternal.shiftCols + (int)settingsInternal.offsetCols < 0)
		{
			return false;
		}

		if (std::max<int>(col + settingsInternal.shiftCols, col) > image.width() - settings.templatePatch.width)
		{
			return false;
		}

		return true;
	}

}
