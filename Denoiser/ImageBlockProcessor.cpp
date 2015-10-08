#include "ImageBlockProcessor.h"

#include <algorithm>
#include "SortedPatchCollection.h"

namespace Denoise
{

	ImageBlockProcessor::ImageBlockProcessor(Image& image) : m_image(image)
	{

	}


	ImageBlockProcessor::~ImageBlockProcessor()
	{

	}


	void ImageBlockProcessor::computeNMostSimilar(const ImagePatch& templatePatch, const Rectangle& imageBlock,
		index_t stepSizeRows, index_t stepSizeCols,
		index_t windowSizeRows, index_t windowSizeCols,
		index_t maxSimilar, float maxDistance,
		int norm,
		std::vector<std::vector<IDX2> >& matchedBlocks,
		index_t numChannelsToUse)
	{
		//make sure we are accessing the full image
		m_image.accessFullImage();

		std::vector<SortedPatchCollection> matchedBlocksSorted;

		//we only allocate as many blocks as we need
		for (index_t i = 0; i < (imageBlock.width() / stepSizeCols) * (imageBlock.height() / stepSizeRows); ++i)
		{
			matchedBlocksSorted.push_back(SortedPatchCollection(maxSimilar));

		}
		//matchedBlocksSorted.resize((imageBlock.width() / stepSizeCols) * (imageBlock.height() / stepSizeRows));

		//do block matching
		int halfWindowSizeRows = windowSizeRows / 2;
		int halfWindowSizeCols = windowSizeCols / 2;

		//Compute in double precision to avoid round-off errors for large images
		std::vector<std::vector<float> > distanceImage(numChannelsToUse);
		std::vector<std::vector<double> > integralImage(numChannelsToUse);

		for (index_t c = 0; c < numChannelsToUse; ++c)
		{
			distanceImage[c].resize(imageBlock.size());
			integralImage[c].resize(imageBlock.size());
		}

		for (int shiftRows = -halfWindowSizeRows; shiftRows <= halfWindowSizeRows; ++shiftRows)
		{
			for (int shiftCols = -halfWindowSizeCols; shiftCols <= halfWindowSizeCols; ++shiftCols)
			{
				for (index_t c = 0; c < numChannelsToUse; ++c)
				{
					//A. Compute Pixel Differences
					for (int row = imageBlock.bottom; row < imageBlock.top; ++row)
					{
						if (row + shiftRows < 0 || row + shiftRows > m_image.height() - 1)
						{
							continue;
						}

						for (int col = imageBlock.left; col < imageBlock.right; ++col)
						{
							if (col + shiftCols < 0
								|| col + shiftCols > m_image.width() - 1)
							{
								continue;
							}

							index_t idx = row * m_image.width() + col;
							index_t compareIdx = (row + shiftRows) * m_image.width() + (col + shiftCols);
							index_t blockIdx = (row - imageBlock.bottom) * imageBlock.width() + (col - imageBlock.left);

							distanceImage[c][blockIdx] = std::pow(m_image.getPixel(c, idx) - m_image.getPixel(c, compareIdx), norm);
						}
					}
				}

				//B. Compute Integral Image
				for (index_t c = 0; c < numChannelsToUse; ++c)
				{
					computeIntegralImage(distanceImage[c], imageBlock, integralImage[c]);
				}

				//C. Evaluate Patch Distances
				for (int row = imageBlock.bottom; row <= imageBlock.top - templatePatch.height; row += stepSizeRows)
				{
					if (row + shiftRows < 0)
					{
						continue;
					}

					if (row + shiftRows > m_image.height() - templatePatch.height)
					{
						continue;
					}

					for (int col = imageBlock.left; col <= imageBlock.right - templatePatch.width; col += stepSizeCols)
					{
						if (col + shiftCols < 0)
						{
							continue;
						}

						if (col + shiftCols > m_image.width() - templatePatch.width)
						{
							continue;
						}

						double distance = 0.0;
						for (index_t c = 0; c < numChannelsToUse; ++c)
						{
							distance += patchDistanceIntegralImage(integralImage[c],
								templatePatch, imageBlock, IDX2(row - imageBlock.bottom, col - imageBlock.left));
						}

						distance /= (double)numChannelsToUse;

						if (distance <= (double)maxDistance)
						{
							matchedBlocksSorted[((row - imageBlock.bottom) / stepSizeRows) * (imageBlock.width() / stepSizeCols) + (col - imageBlock.left) / stepSizeCols].insertPatch(
								IDX2(row + shiftRows, col + shiftCols, distance));
						}
					}
				}

				//Evaluate right and top borders (to avoid any black seams in the image)
				
			}
		}

		distanceImage.clear();
		integralImage.clear();

		matchedBlocks.resize((m_image.width() / stepSizeCols) * (m_image.height() / stepSizeRows));

		for (int row = imageBlock.bottom + 1; row < imageBlock.top - templatePatch.height; row += stepSizeRows)
		{
			for (int col = imageBlock.left + 1; col < imageBlock.right - templatePatch.width; col += stepSizeCols)
			{
				matchedBlocks[(row / stepSizeRows) * (m_image.width() / stepSizeCols) + (col / stepSizeCols)] =
					matchedBlocksSorted[((row - imageBlock.bottom) / stepSizeRows) * (imageBlock.width() / stepSizeCols) + (col - imageBlock.left) / stepSizeCols].getPatches();
			}
		}

		matchedBlocksSorted.clear();
	}

	void ImageBlockProcessor::computeIntegralImage(const std::vector<float>& pixels, const Rectangle& imageBlock,
		std::vector<double>& integralImage)
	{
		//see e.g. http://www.ipol.im/pub/art/2014/57/article.pdf for details on this recurrence-relation
		integralImage[0] = (double)pixels[0];

		for (int col = std::max(imageBlock.left - 1, 1); col < imageBlock.width(); ++col)
		{
			integralImage[col] = integralImage[col - 1] + (double)pixels[col];
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

	void ImageBlockProcessor::computeNMostSimilarNaive(std::vector<IDX2>& matchedBlocks, const IDX2& position, const ImagePatch& templatePatch,
		index_t windowSizeRows, index_t windowSizeCols,
		index_t maxSimilar, float maxDistance, int norm)
	{
		//do block matching
		int halfWindowSizeRows = windowSizeRows / 2;
		int halfWindowSizeCols = windowSizeCols / 2;

		SortedPatchCollection foundMatches(maxSimilar);

		for (int shiftRows = -halfWindowSizeRows; shiftRows <= halfWindowSizeRows; ++shiftRows)
		{
			for (int shiftCols = -halfWindowSizeCols; shiftCols <= halfWindowSizeCols; ++shiftCols)
			{
				ImagePatch currentPatch;
				currentPatch.col = position.col + shiftCols;
				currentPatch.row = position.row + shiftRows;
				currentPatch.height = templatePatch.height;
				currentPatch.width = templatePatch.width;
				float distance = (m_image.blockMatch_Naive(templatePatch, currentPatch, 0, 2)
					+ m_image.blockMatch_Naive(templatePatch, currentPatch, 1, 2)
					+ m_image.blockMatch_Naive(templatePatch, currentPatch, 2, 2)) / 3.0f;

				if (distance <= maxDistance)
				{
					//foundMatches.insertPatch(IDX2(currentPatch.row, currentPatch.col, distance));
					matchedBlocks.push_back(IDX2(currentPatch.row, currentPatch.col, distance));
				}
			}
		}

		std::sort(matchedBlocks.begin(), matchedBlocks.end());
		if (matchedBlocks.size() > maxSimilar)
		{
			matchedBlocks.erase(matchedBlocks.begin() + maxSimilar, matchedBlocks.end());
		}
		//matchedBlocks = foundMatches.getPatches();
	}

}