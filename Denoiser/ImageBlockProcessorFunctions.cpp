#include "ImageBlockProcessorFunctions.h"

#include "SortedPatchCollection.h"
#include <numeric>
#include <algorithm>
#include <iostream>

namespace Denoise
{
	void computeBlockMatchingForSpecificShifts(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<double> >& distanceImage,
		std::vector<std::vector<double> >& integralImage,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal)
	{
		//do block matching
		int halfWindowSizeRows = settings.windowSizeRows / 2;
		int halfWindowSizeCols = settings.windowSizeCols / 2;

		for (index_t c = 0; c < settings.numChannelsToUse; ++c)
		{
			//A. Compute Pixel Differences
			for (int row = settingsInternal.accessibleImageBlock.bottom; row < settingsInternal.accessibleImageBlock.top; ++row)
			{
				if (row + settingsInternal.shiftRows < 0 || row + settingsInternal.shiftRows > image.height() - 1)
				{
					continue;
				}

				for (int col = settingsInternal.accessibleImageBlock.left; col < settingsInternal.accessibleImageBlock.right; ++col)
				{
					if (col + settingsInternal.shiftCols < 0
						|| col + settingsInternal.shiftCols > image.width() - 1)
					{
						continue;
					}

					index_t idx = row * image.width() + col;
					index_t compareIdx = (row + settingsInternal.shiftRows) * image.width() + (col + settingsInternal.shiftCols);
					index_t blockIdx = (row - settingsInternal.accessibleImageBlock.bottom)
						* settingsInternal.accessibleImageBlock.width() + (col - settingsInternal.accessibleImageBlock.left);

					distanceImage[c][blockIdx] = std::pow(image.getPixel(c, idx) - image.getPixel(c, compareIdx), settings.norm);
				}
			}
		}

		//B. Compute Integral Image
		for (index_t c = 0; c < settings.numChannelsToUse; ++c)
		{
			computeIntegralImage(distanceImage[c], settingsInternal.accessibleImageBlockShifted, integralImage[c]);
		}

		//C. Evaluate Patch Distances
		for (int row = settings.imageBlock.bottom; row <= settings.imageBlock.top - settings.templatePatch.height; row += settings.stepSizeRows)
		{
			if (row + settingsInternal.offsetRows + settingsInternal.shiftRows < 0)
			{
				continue;
			}

			if (row + settingsInternal.shiftRows > image.height() - settings.templatePatch.height)
			{
				continue;
			}

			for (int col = settings.imageBlock.left; col <= settings.imageBlock.right - settings.templatePatch.width; col += settings.stepSizeCols)
			{
				if (col + settingsInternal.shiftCols + settingsInternal.offsetCols < 0)
				{
					continue;
				}

				if (col + settingsInternal.shiftCols > image.width() - settings.templatePatch.width)
				{
					continue;
				}

				double distance = 0.0;
				for (index_t c = 0; c < settings.numChannelsToUse; ++c)
				{
					distance += patchDistanceIntegralImage(integralImage[c],
						settings.templatePatch, settingsInternal.accessibleImageBlock,
						IDX2(row - settings.imageBlock.bottom + settingsInternal.offsetRows, col - settings.imageBlock.left + settingsInternal.offsetCols));
				}

				distance /= (double)settings.numChannelsToUse;

				if (settingsInternal.shiftCols == 0 && settingsInternal.shiftRows == 0)
				{
					distance = -100.0f;
				}

				if (distance <= (double)settings.maxDistance)
				{
					matchedBlocksSorted[((row - settings.imageBlock.bottom) / settings.stepSizeRows)
						* (settings.imageBlock.width() / settings.stepSizeCols + 1)
						+ (col - settings.imageBlock.left) / settings.stepSizeCols].insertPatch(
						IDX2(row + settingsInternal.shiftRows, col + settingsInternal.shiftCols, distance));
				}
			}
		}

		if (true)
		{
			if (settings.stepSizeRows > 1)
			{
				int row = settings.imageBlock.top - settings.templatePatch.height;
				for (int col = settings.imageBlock.left; col <= settings.imageBlock.right - settings.templatePatch.width; col += settings.stepSizeCols)
				{
					if (col + settingsInternal.shiftCols + settingsInternal.offsetCols < 0)
					{
						continue;
					}

					if (row + settingsInternal.shiftRows > image.height() - settings.templatePatch.height)
					{
						continue;
					}

					if (col + settingsInternal.shiftCols > image.width() - settings.templatePatch.width)
					{
						continue;
					}

					double distance = 0.0;
					for (index_t c = 0; c < settings.numChannelsToUse; ++c)
					{
						distance += patchDistanceIntegralImage(integralImage[c],
							settings.templatePatch, settingsInternal.accessibleImageBlock,
							IDX2(row - settings.imageBlock.bottom + settingsInternal.offsetRows, col - settings.imageBlock.left + settingsInternal.offsetCols));
					}

					distance /= (double)settings.numChannelsToUse;

					if (settingsInternal.shiftCols == 0 && settingsInternal.shiftRows == 0)
					{
						distance = -100.0f;
					}

					if (distance <= (double)settings.maxDistance)
					{
						matchedBlocksSorted[(settings.imageBlock.height() / settings.stepSizeRows)
							* (settings.imageBlock.width() / settings.stepSizeCols + 1)
							+ (col - settings.imageBlock.left) / settings.stepSizeCols].insertPatch(
							IDX2(row + settingsInternal.shiftRows, col + settingsInternal.shiftCols, distance));
					}
				}
			}

			if (settings.stepSizeCols > 1)
			{
				for (int row = settings.imageBlock.bottom; row <= settings.imageBlock.top - settings.templatePatch.height; row += settings.stepSizeRows)
				{
					if (row + settingsInternal.shiftRows + settingsInternal.offsetRows < 0)
					{
						continue;
					}

					int col = settings.imageBlock.right - settings.templatePatch.width;

					if (row + settingsInternal.shiftRows > image.height() - settings.templatePatch.height)
					{
						continue;
					}

					if (col + settingsInternal.shiftCols > image.width() - settings.templatePatch.width)
					{
						continue;
					}

					double distance = 0.0;
					for (index_t c = 0; c < settings.numChannelsToUse; ++c)
					{
						distance += patchDistanceIntegralImage(integralImage[c],
							settings.templatePatch, settingsInternal.accessibleImageBlock,
							IDX2(row - settings.imageBlock.bottom + settingsInternal.offsetRows, col - settings.imageBlock.left + settingsInternal.offsetCols));
					}

					distance /= (double)settings.numChannelsToUse;

					if (settingsInternal.shiftCols == 0 && settingsInternal.shiftRows == 0)
					{
						distance = -100.0f;
					}

					if (distance <= (double)settings.maxDistance)
					{
						matchedBlocksSorted[((row - settings.imageBlock.bottom) / settings.stepSizeRows) * (settings.imageBlock.width() / settings.stepSizeCols + 1) + (settings.imageBlock.width() / settings.stepSizeCols + 1)].insertPatch(
							IDX2(row + settingsInternal.shiftRows, col + settingsInternal.shiftCols, distance));
					}
				}
			}

			if (settings.stepSizeCols > 1 || settings.stepSizeRows > 1)
			{
				int row = settings.imageBlock.top - settings.templatePatch.height;
				int col = settings.imageBlock.right - settings.templatePatch.width;

				if (row + settingsInternal.shiftRows > image.height() - settings.templatePatch.height)
				{
					return;
				}

				if (col + settingsInternal.shiftCols > image.width() - settings.templatePatch.width)
				{
					return;
				}

				double distance = 0.0;
				for (index_t c = 0; c < settings.numChannelsToUse; ++c)
				{
					distance += patchDistanceIntegralImage(integralImage[c],
						settings.templatePatch, settingsInternal.accessibleImageBlock,
						IDX2(row - settings.imageBlock.bottom + settingsInternal.offsetRows, col - settings.imageBlock.left + settingsInternal.offsetCols));
				}

				distance /= (double)settings.numChannelsToUse;

				if (settingsInternal.shiftCols == 0 && settingsInternal.shiftRows == 0)
				{
					distance = -100.0f;
				}

				if (distance <= (double)settings.maxDistance)
				{
					matchedBlocksSorted[matchedBlocksSorted.size() - 1].insertPatch(
						IDX2(row + settingsInternal.shiftRows, col + settingsInternal.shiftCols, distance));
				}
			}
		}
	}

	void computeBlockMatchingForSpecificShifts_doNotComputeIntegralImage(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<double> >& distanceImage,
		std::vector<std::vector<double> >& integralImage,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal)
	{
		//do block matching
		int halfWindowSizeRows = settings.windowSizeRows / 2;
		int halfWindowSizeCols = settings.windowSizeCols / 2;

		//C. Evaluate Patch Distances
		for (int row = settings.imageBlock.bottom; row <= settings.imageBlock.top - settings.templatePatch.height; row += settings.stepSizeRows)
		{
			if (row + settingsInternal.offsetRows + settingsInternal.shiftRows < 0)
			{
				continue;
			}

			if (row + settingsInternal.shiftRows > image.height() - settings.templatePatch.height)
			{
				continue;
			}

			for (int col = settings.imageBlock.left; col <= settings.imageBlock.right - settings.templatePatch.width; col += settings.stepSizeCols)
			{
				if (col + settingsInternal.shiftCols + settingsInternal.offsetCols < 0)
				{
					continue;
				}

				if (col + settingsInternal.shiftCols > image.width() - settings.templatePatch.width)
				{
					continue;
				}

				double distance = 0.0;
				for (index_t c = 0; c < settings.numChannelsToUse; ++c)
				{
					distance += patchDistanceIntegralImage(integralImage[c],
						settings.templatePatch, settingsInternal.accessibleImageBlock,
						IDX2(row - settings.imageBlock.bottom + settingsInternal.offsetRows, col - settings.imageBlock.left + settingsInternal.offsetCols));
				}

				distance /= (double)settings.numChannelsToUse;

				if (settingsInternal.shiftCols == 0 && settingsInternal.shiftRows == 0)
				{
					distance = -100.0f;
				}

				if (distance <= (double)settings.maxDistance)
				{
					matchedBlocksSorted[((row - settings.imageBlock.bottom) / settings.stepSizeRows)
						* (settings.imageBlock.width() / settings.stepSizeCols + 1)
						+ (col - settings.imageBlock.left) / settings.stepSizeCols].insertPatch(
						IDX2(row + settingsInternal.shiftRows, col + settingsInternal.shiftCols, distance));
				}
			}
		}

		if (true)
		{
			if (settings.stepSizeRows > 1)
			{
				int row = settings.imageBlock.top - settings.templatePatch.height;
				for (int col = settings.imageBlock.left; col <= settings.imageBlock.right - settings.templatePatch.width; col += settings.stepSizeCols)
				{
					if (col + settingsInternal.shiftCols + settingsInternal.offsetCols < 0)
					{
						continue;
					}

					if (row + settingsInternal.shiftRows > image.height() - settings.templatePatch.height)
					{
						continue;
					}

					if (col + settingsInternal.shiftCols > image.width() - settings.templatePatch.width)
					{
						continue;
					}

					double distance = 0.0;
					for (index_t c = 0; c < settings.numChannelsToUse; ++c)
					{
						distance += patchDistanceIntegralImage(integralImage[c],
							settings.templatePatch, settingsInternal.accessibleImageBlock,
							IDX2(row - settings.imageBlock.bottom + settingsInternal.offsetRows, col - settings.imageBlock.left + settingsInternal.offsetCols));
					}

					distance /= (double)settings.numChannelsToUse;

					if (settingsInternal.shiftCols == 0 && settingsInternal.shiftRows == 0)
					{
						distance = -100.0f;
					}

					if (distance <= (double)settings.maxDistance)
					{
						matchedBlocksSorted[(settings.imageBlock.height() / settings.stepSizeRows)
							* (settings.imageBlock.width() / settings.stepSizeCols + 1)
							+ (col - settings.imageBlock.left) / settings.stepSizeCols].insertPatch(
							IDX2(row + settingsInternal.shiftRows, col + settingsInternal.shiftCols, distance));
					}
				}
			}

			if (settings.stepSizeCols > 1)
			{
				for (int row = settings.imageBlock.bottom; row <= settings.imageBlock.top - settings.templatePatch.height; row += settings.stepSizeRows)
				{
					if (row + settingsInternal.shiftRows + settingsInternal.offsetRows < 0)
					{
						continue;
					}

					int col = settings.imageBlock.right - settings.templatePatch.width;

					if (row + settingsInternal.shiftRows > image.height() - settings.templatePatch.height)
					{
						continue;
					}

					if (col + settingsInternal.shiftCols > image.width() - settings.templatePatch.width)
					{
						continue;
					}

					double distance = 0.0;
					for (index_t c = 0; c < settings.numChannelsToUse; ++c)
					{
						distance += patchDistanceIntegralImage(integralImage[c],
							settings.templatePatch, settingsInternal.accessibleImageBlock,
							IDX2(row - settings.imageBlock.bottom + settingsInternal.offsetRows, col - settings.imageBlock.left + settingsInternal.offsetCols));
					}

					distance /= (double)settings.numChannelsToUse;

					if (settingsInternal.shiftCols == 0 && settingsInternal.shiftRows == 0)
					{
						distance = -100.0f;
					}

					if (distance <= (double)settings.maxDistance)
					{
						matchedBlocksSorted[((row - settings.imageBlock.bottom) / settings.stepSizeRows) * (settings.imageBlock.width() / settings.stepSizeCols + 1) + (settings.imageBlock.width() / settings.stepSizeCols + 1)].insertPatch(
							IDX2(row + settingsInternal.shiftRows, col + settingsInternal.shiftCols, distance));
					}
				}
			}

			if (settings.stepSizeCols > 1 || settings.stepSizeRows > 1)
			{
				int row = settings.imageBlock.top - settings.templatePatch.height;
				int col = settings.imageBlock.right - settings.templatePatch.width;

				if (row + settingsInternal.shiftRows > image.height() - settings.templatePatch.height)
				{
					return;
				}

				if (col + settingsInternal.shiftCols > image.width() - settings.templatePatch.width)
				{
					return;
				}

				double distance = 0.0;
				for (index_t c = 0; c < settings.numChannelsToUse; ++c)
				{
					distance += patchDistanceIntegralImage(integralImage[c],
						settings.templatePatch, settingsInternal.accessibleImageBlock,
						IDX2(row - settings.imageBlock.bottom + settingsInternal.offsetRows, col - settings.imageBlock.left + settingsInternal.offsetCols));
				}

				distance /= (double)settings.numChannelsToUse;

				if (settingsInternal.shiftCols == 0 && settingsInternal.shiftRows == 0)
				{
					distance = -100.0f;
				}

				if (distance <= (double)settings.maxDistance)
				{
					matchedBlocksSorted[matchedBlocksSorted.size() - 1].insertPatch(
						IDX2(row + settingsInternal.shiftRows, col + settingsInternal.shiftCols, distance));
				}
			}
		}
	}

	void computeBlockMatchingForSpecificShifts_doNotComputeIntegralImageBlock(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<std::vector<double> > >& distanceImage,
		std::vector<std::vector<std::vector<double> > >& integralImage,
		const std::vector<std::pair<int, int> >& shifts,
		index_t startIdx,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal)
	{
		//do block matching
		int halfWindowSizeRows = settings.windowSizeRows / 2;
		int halfWindowSizeCols = settings.windowSizeCols / 2;
		for (index_t i = 0; i < shifts.size(); ++i)
		{
			int shiftRows = shifts[i].first;
			int shiftCols = shifts[i].second;

			index_t localStepSizeRows = settings.stepSizeRows;
			index_t localStepSizeCols = settings.stepSizeCols;

			//C. Evaluate Patch Distances
			for (int row = settings.imageBlock.bottom; row < settings.imageBlock.top; row += localStepSizeRows)
			{
				if (settingsInternal.iterateAtBorders)
				{
					if (row + localStepSizeRows * 2 >= settings.imageBlock.top)
					{
						localStepSizeRows = 1;
					}
				}

				if (row + settingsInternal.offsetRows + shiftRows < 0)
				{
					continue;
				}


				if (row + shiftRows > image.height() - settings.templatePatch.height + 0)
				{
					continue;
				}

				localStepSizeCols = settings.stepSizeCols;

				for (int col = settings.imageBlock.left; col < settings.imageBlock.right; col += localStepSizeCols)
				{
					if (col + settings.stepSizeCols * 2 >= settings.imageBlock.right)
					{
						localStepSizeCols = 1;
					}

					if (col + shiftCols + settingsInternal.offsetCols < 0)
					{
						continue;
					}

					if (shiftRows == 0 || shiftCols == 0)
					{
						if (col + shiftCols > image.width() - settings.templatePatch.width + 1)
						{
							continue;
						}
					}
					else
					{
						if (col + shiftCols > image.width() - settings.templatePatch.width + 0)
						{
							continue;
						}
					}

					double distance = 0.0;
					for (index_t c = 0; c < settings.numChannelsToUse; ++c)
					{
						distance += patchDistanceIntegralImage(integralImage[startIdx + i][c],
							settings.templatePatch, settingsInternal.accessibleImageBlock,
							IDX2(row  + settingsInternal.offsetRows, col + settingsInternal.offsetCols));
					}

					distance /= (double)settings.numChannelsToUse;

					if (shiftCols == 0 && shiftRows == 0)
					{
						distance = -100.0f;
					}

					index_t blockRow = std::round((float)row / (float)settings.stepSizeRows);
					index_t blockCol = std::round((float)col / (float)settings.stepSizeCols);

					index_t blockWidth = std::ceil((float)settings.imageBlock.width() / (float)settings.stepSizeCols);

					if (distance <= (double)settings.maxDistance)
					{
						matchedBlocksSorted[blockRow * blockWidth + blockCol].insertPatch(
							IDX2(row + shiftRows, col + shiftCols, distance));
					}
				}
			}
		}
	}

	void computeIntegralImageForSpecificShifts(const Image& image,
		std::vector<SortedPatchCollection>& matchedBlocksSorted,
		std::vector<std::vector<double> >& distanceImage,
		std::vector<std::vector<double> >& integralImage,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal)
	{
		//do block matching
		int halfWindowSizeRows = settings.windowSizeRows / 2;
		int halfWindowSizeCols = settings.windowSizeCols / 2;

		for (index_t c = 0; c < settings.numChannelsToUse; ++c)
		{
			//A. Compute Pixel Differences
			for (int row = settingsInternal.accessibleImageBlock.bottom; row < settingsInternal.accessibleImageBlock.top; ++row)
			{
				if (row + settingsInternal.shiftRows < 0 || row + settingsInternal.shiftRows > image.height() - 1)
				{
					continue;
				}

				for (int col = settingsInternal.accessibleImageBlock.left; col < settingsInternal.accessibleImageBlock.right; ++col)
				{
					if (col + settingsInternal.shiftCols < 0
						|| col + settingsInternal.shiftCols > image.width() - 1)
					{
						continue;
					}

					index_t idx = row * image.width() + col;
					index_t compareIdx = (row + settingsInternal.shiftRows) * image.width() + (col + settingsInternal.shiftCols);
					index_t blockIdx = (row - settingsInternal.accessibleImageBlock.bottom)
						* settingsInternal.accessibleImageBlock.width() + (col - settingsInternal.accessibleImageBlock.left);

					distanceImage[c][blockIdx] = std::pow(image.getPixel(c, idx) - image.getPixel(c, compareIdx), settings.norm);
				}
			}
		}

		//B. Compute Integral Image
		for (index_t c = 0; c < settings.numChannelsToUse; ++c)
		{
			computeIntegralImage(distanceImage[c], settingsInternal.accessibleImageBlockShifted, integralImage[c]);
		}
	}

	void computeIntegralImageForSpecificShiftsBlock(const Image& image,
		std::vector<std::vector<std::vector<double> > >& distanceImage,
		std::vector<std::vector<std::vector<double> > >& integralImage,
		const std::vector<std::pair<int, int> >& shifts,
		index_t startIdx,
		const ImageBlockProcessorSettings& settings,
		const ImageBlockProcessorSettingsInternal& settingsInternal)
	{
		//do block matching
		int halfWindowSizeRows = settings.windowSizeRows / 2;
		int halfWindowSizeCols = settings.windowSizeCols / 2;

		for (index_t i = 0; i < shifts.size(); ++i)
		{
			int shiftRows = shifts[i].first;
			int shiftCols = shifts[i].second;
			
			for (index_t c = 0; c < settings.numChannelsToUse; ++c)
			{
				//A. Compute Pixel Differences
				for (int row = settingsInternal.accessibleImageBlock.bottom; row < settingsInternal.accessibleImageBlock.top; ++row)
				{
					if (row + shiftRows < 0 || row + shiftRows > image.height() - 1)
					{
						continue;
					}

					for (int col = settingsInternal.accessibleImageBlock.left; col < settingsInternal.accessibleImageBlock.right; ++col)
					{
						if (col + shiftCols < 0
							|| col + shiftCols > image.width() - 1)
						{
							continue;
						}

						index_t idx = row * image.width() + col;
						index_t compareIdx = (row + shiftRows) * image.width() + (col + shiftCols);
						index_t blockIdx = (row - settingsInternal.accessibleImageBlock.bottom)
							* settingsInternal.accessibleImageBlock.width() + (col - settingsInternal.accessibleImageBlock.left);

						distanceImage[startIdx + i][c][blockIdx] = std::pow(image.getPixel(c, idx) - image.getPixel(c, compareIdx), settings.norm);
					}
				}
			}

			//B. Compute Integral Image
			for (index_t c = 0; c < settings.numChannelsToUse; ++c)
			{
				computeIntegralImage(distanceImage[startIdx + i][c], settingsInternal.accessibleImageBlockShifted, integralImage[startIdx + i][c]);
			}
		}
	}


	void computeIntegralImage(const std::vector<double>& pixels, const Rectangle& imageBlock,
		std::vector<double>& integralImage)
	{
		//see e.g. http://www.ipol.im/pub/art/2014/57/article.pdf for details on this recurrence-relation
		integralImage[0] = pixels[0];

		for (int col = std::max(imageBlock.left - 1, 1); col < imageBlock.width(); ++col)
		{
			integralImage[col] = integralImage[col - 1] + pixels[col];
		}

		for (int row = std::max(imageBlock.bottom - 1, 1); row < imageBlock.height(); ++row)
		{
			long double s = (long double)pixels[row * imageBlock.width()];
			integralImage[row * imageBlock.width()] = integralImage[(row - 1) * imageBlock.width()] + s;
			for (int col = 1; col < imageBlock.width(); ++col)
			{
				s += (long double)pixels[row * imageBlock.width() + col];
				integralImage[row * imageBlock.width() + col] = integralImage[(row - 1) * imageBlock.width() + col] + s;
			}
		}
	}
}