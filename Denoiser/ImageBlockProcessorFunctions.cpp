#include "ImageBlockProcessorFunctions.h"

#include "SortedPatchCollection.h"
#include <numeric>
#include <algorithm>
#include <iostream>

#include "lodepng.h"

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

		if (settingsInternal.shiftCols == 1 && settingsInternal.shiftRows == 0)
		{
			printBuffer(integralImage[0], image.height(), image.width(), "SequentialIntegral.png");
		}

		int blockRow = 0;
		int blockCol = 0;

		//C. Evaluate Patch Distances
		int row;
		for (row = settings.imageBlock.bottom; row <= settings.imageBlock.top - settings.templatePatch.height; row += settings.stepSizeRows)
		{
			if (!checkRow(image, settings, settingsInternal, row))
			{
				++blockRow;
				continue;
			}

			int col;
			for (col = settings.imageBlock.left; col <= settings.imageBlock.right - settings.templatePatch.width; col += settings.stepSizeCols)
			{
				if (!checkCol(image, settings, settingsInternal, col))
				{
					++blockCol;
					continue;
				}

				double distance = computeDistanceForShift(integralImage, settings, settingsInternal, row, col);

				if (distance <= (double)settings.maxDistance)
				{
					matchedBlocksSorted[blockRow
						* (settings.imageBlock.width() / settings.stepSizeCols + 1)
						+ blockCol].insertPatch(
						IDX2(row + settingsInternal.shiftRows, col + settingsInternal.shiftCols, distance));
				}
				++blockCol;
			}
			if (col != settings.imageBlock.right - settings.templatePatch.width)
			{
				col = settings.imageBlock.right - settings.templatePatch.width;

				if (!checkCol(image, settings, settingsInternal, col))
				{
					continue;
				}

				double distance = computeDistanceForShift(integralImage, settings, settingsInternal, row, col);

				if (distance <= (double)settings.maxDistance)
				{
					matchedBlocksSorted[blockRow
						* (settings.imageBlock.width() / settings.stepSizeCols + 1)
						+ blockCol].insertPatch(
						IDX2(row + settingsInternal.shiftRows, col + settingsInternal.shiftCols, distance));
				}
			}

			blockCol = 0;
			++blockRow;
		}
		if (row != settings.imageBlock.top - settings.templatePatch.height)
		{
			row = settings.imageBlock.top - settings.templatePatch.height;

			if (!checkRow(image, settings, settingsInternal, row))
			{
				return;
			}

			int col;
			for (col = settings.imageBlock.left; col <= settings.imageBlock.right - settings.templatePatch.width; col += settings.stepSizeCols)
			{
				if (col + settingsInternal.shiftCols + settingsInternal.offsetCols < 0)
				{
					++blockCol;
					continue;
				}

				if (col + settingsInternal.shiftCols > image.width() - settings.templatePatch.width)
				{
					++blockCol;
					continue;
				}

				double distance = computeDistanceForShift(integralImage, settings, settingsInternal, row, col);

				if (distance <= (double)settings.maxDistance)
				{
					matchedBlocksSorted[blockRow
						* (settings.imageBlock.width() / settings.stepSizeCols + 1)
						+ blockCol].insertPatch(
						IDX2(row + settingsInternal.shiftRows, col + settingsInternal.shiftCols, distance));
				}
				++blockCol;
			}
			if (col != settings.imageBlock.right - settings.templatePatch.width)
			{
				col = settings.imageBlock.right - settings.templatePatch.width;

				if (!checkCol(image, settings, settingsInternal, col))
				{
					return;
				}

				double distance = computeDistanceForShift(integralImage, settings, settingsInternal, row, col);

				if (distance <= (double)settings.maxDistance)
				{
					matchedBlocksSorted[blockRow
						* (settings.imageBlock.width() / settings.stepSizeCols + 1)
						+ blockCol].insertPatch(
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
		ImageBlockProcessorSettings globalSettings = settings;
		globalSettings.imageBlock = Rectangle(0, image.width(), image.height(), 0);

		//do block matching
		int halfWindowSizeRows = settings.windowSizeRows / 2;
		int halfWindowSizeCols = settings.windowSizeCols / 2;
		for (index_t i = 0; i < shifts.size(); ++i)
		{
			ImageBlockProcessorSettingsInternal localSettings = settingsInternal;
			localSettings.shiftRows = shifts[i].first;
			localSettings.shiftCols = shifts[i].second;

			int blockRow = settings.imageBlock.bottom / settings.stepSizeRows;
			int blockCol = 0;

			//C. Evaluate Patch Distances
			int row;
			for (row = settings.imageBlock.bottom; row <= settings.imageBlock.top - 0; row += settings.stepSizeRows)
			{
				if (!checkRow(image, settings, localSettings, row))
				{
					++blockRow;
					continue;
				}

				int col;
				for (col = settings.imageBlock.left; col <= settings.imageBlock.right - 0; col += settings.stepSizeCols)
				{
					if (!checkCol(image, settings, localSettings, col))
					{
						++blockCol;
						continue;
					}

					double distance = computeDistanceForShift(integralImage[i], globalSettings, localSettings, row, col);

					if (distance <= (double)settings.maxDistance)
					{
						matchedBlocksSorted[blockRow
							* (settings.imageBlock.width() / settings.stepSizeCols + 1)
							+ blockCol].insertPatch(
							IDX2(row + localSettings.shiftRows, col + localSettings.shiftCols, distance));
					}
					++blockCol;
				}
				if (col != settings.imageBlock.right - settings.templatePatch.width)
				{
					col = settings.imageBlock.right - settings.templatePatch.width;

					if (!checkCol(image, settings, localSettings, col))
					{
						continue;
					}

					double distance = computeDistanceForShift(integralImage[i], globalSettings, localSettings, row, col);

					if (distance <= (double)settings.maxDistance)
					{
						matchedBlocksSorted[blockRow
							* (settings.imageBlock.width() / settings.stepSizeCols + 1)
							+ blockCol].insertPatch(
							IDX2(row + localSettings.shiftRows, col + localSettings.shiftCols, distance));
					}
				}

				blockCol = 0;
				++blockRow;
			}

			if (!localSettings.iterateAtBorders)
			{
				continue;
			}

			if (row != settings.imageBlock.top - settings.templatePatch.height)
			{
				row = settings.imageBlock.top - settings.templatePatch.height;

				if (!checkRow(image, settings, localSettings, row))
				{
					continue;
				}

				int col;
				for (col = settings.imageBlock.left; col <= settings.imageBlock.right - settings.templatePatch.width; col += settings.stepSizeCols)
				{
					if (col + localSettings.shiftCols + localSettings.offsetCols < 0)
					{
						++blockCol;
						continue;
					}

					if (col + localSettings.shiftCols > image.width() - settings.templatePatch.width)
					{
						++blockCol;
						continue;
					}

					double distance = computeDistanceForShift(integralImage[i], globalSettings, localSettings, row, col);

					if (distance <= (double)settings.maxDistance)
					{
						matchedBlocksSorted[blockRow
							* (settings.imageBlock.width() / settings.stepSizeCols + 1)
							+ blockCol].insertPatch(
							IDX2(row + localSettings.shiftRows, col + localSettings.shiftCols, distance));
					}
					++blockCol;
				}
				if (col != settings.imageBlock.right - settings.templatePatch.width)
				{
					col = settings.imageBlock.right - settings.templatePatch.width;

					if (!checkCol(image, settings, localSettings, col))
					{
						continue;
					}

					double distance = computeDistanceForShift(integralImage[i], globalSettings, localSettings, row, col);

					if (distance <= (double)settings.maxDistance)
					{
						matchedBlocksSorted[blockRow
							* (settings.imageBlock.width() / settings.stepSizeCols + 1)
							+ blockCol].insertPatch(
							IDX2(row + localSettings.shiftRows, col + localSettings.shiftCols, distance));
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
			
			//A. Compute Integral Image
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


	void printBuffer(const std::vector<double>& pixels, int height, int width, const std::string& fileName)
	{
		std::vector<double> normalisedPixels(height * width);
		double maxValue = -10000000.0;

		for (int i = 0; i < width * height; ++i)
		{
			if (pixels[i] > maxValue)
			{
				maxValue = pixels[i];
			}
		}

		for (int i = 0; i < width * height; ++i)
		{
			normalisedPixels[i] = (pixels[i] / maxValue) * 255.0;
		}

		std::vector<unsigned char> png;

		std::vector<unsigned char> rawImage;
		rawImage.resize(width * height * 4);

		for (index_t row = 0; row < height; ++row)
		{
			for (index_t col = 0; col < width; ++col)
			{
				for (index_t c = 0; c < 4; ++c)
				{
					index_t i = (row * width + col) * 4;
					if (c == 3)
					{
						rawImage[i + c] = (unsigned char)255;
					}
					else
					{
						rawImage[i + c] = (unsigned char)normalisedPixels[row * width + col];
					}
				}
			}
		}

		unsigned error = lodepng::encode(png, rawImage, width, height);
		if (!error) lodepng::save_file(png, fileName);

		//if there's an error, display it
		if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	}
}