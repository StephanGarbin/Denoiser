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
		size_t stepSizeRows, size_t stepSizeCols,
		size_t windowSizeRows, size_t windowSizeCols,
		size_t maxSimilar, float maxDistance,
		int norm,
		std::vector<std::vector<IDX2> >& matchedBlocks,
		size_t numChannelsToUse)
	{
		//make sure we are accessing the full image
		m_image.accessFullImage();

		std::vector<SortedPatchCollection> matchedBlocksSorted;

		//we only allocate as many blocks as we need
		matchedBlocksSorted.resize((imageBlock.width() / stepSizeCols) * (imageBlock.height() / stepSizeRows));

		//do block matching
		int halfWindowSizeRows = windowSizeRows / 2;
		int halfWindowSizeCols = windowSizeCols / 2;

		//Compute in double precision to avoid round-off errors for large images
		std::vector<std::vector<double> > distanceImage(numChannelsToUse);
		std::vector<std::vector<double> > integralImage(numChannelsToUse);

		for (size_t c = 0; c < numChannelsToUse; ++c)
		{
			distanceImage[c].resize(imageBlock.size());
			integralImage[c].resize(imageBlock.size());
		}

		for (int shiftRows = -halfWindowSizeRows; shiftRows <= halfWindowSizeRows; ++shiftRows)
		{
			for (int shiftCols = -halfWindowSizeCols; shiftCols <= halfWindowSizeCols; ++shiftCols)
			{
				for (size_t c = 0; c < numChannelsToUse; ++c)
				{
					//A. Compute Pixel Differences
					for (int row = imageBlock.bottom; row < imageBlock.top; ++row)
					{
						for (int col = imageBlock.left; col < imageBlock.right; ++col)
						{
							int idx = row * m_image.width() + col;
							int compareIdx = (row + shiftRows) * m_image.width() + (col + shiftCols);
							int blockIdx = (row - imageBlock.bottom) * imageBlock.width() + (col - imageBlock.left);
							if (row + shiftRows < 0
								|| col + shiftCols < 0
								|| row + shiftRows > m_image.height() - 1
								|| col + shiftCols > m_image.width() - 1)
							{
								distanceImage[c][blockIdx] = 0.0f;
							}
							else
							{
								distanceImage[c][blockIdx] = std::pow((double)m_image.getPixel(c, idx) - (double)m_image.getPixel(c, compareIdx), norm);
							}
						}
					}
				}

				//B. Compute Integral Image
				for (size_t c = 0; c < numChannelsToUse; ++c)
				{
					computeIntegralImage(distanceImage[c], imageBlock, integralImage[c]);
				}

				//C. Evaluate Patch Distances
				for (int row = imageBlock.bottom + 1; row < imageBlock.top - templatePatch.height; row += stepSizeRows)
				{
					for (int col = imageBlock.left + 1; col < imageBlock.right - templatePatch.width; col += stepSizeCols)
					{
						if (row  + shiftRows < 0
							|| col  + shiftCols < 0)
						{
							continue;
						}

						if (row + shiftRows >= m_image.height() - templatePatch.height
							|| col + shiftCols >= m_image.width() - templatePatch.width)
						{
							continue;
						}

						double distance = 0.0;
						for (size_t c = 0; c < numChannelsToUse; ++c)
						{
							distance += patchDistanceIntegralImage(integralImage[c],
								templatePatch, imageBlock, IDX2(row - imageBlock.bottom, col - imageBlock.left));
						}

						distance /= (double)numChannelsToUse;

						if (distance <= (double)maxDistance)
						{
							matchedBlocksSorted[(row - imageBlock.bottom) * imageBlock.width() + col - imageBlock.left].insertPatch32(
								IDX2(row + shiftRows, col + shiftCols, distance));
						}
					}
				}
			}
		}

		distanceImage.clear();
		integralImage.clear();

		matchedBlocks.resize((m_image.width() / stepSizeCols) * (m_image.height() / stepSizeRows));

		for (int row = imageBlock.bottom; row < imageBlock.top; row += stepSizeRows)
		{
			for (int col = imageBlock.left; col < imageBlock.right; col += stepSizeCols)
			{
				matchedBlocks[(row / stepSizeRows) * (m_image.width() / stepSizeCols) + (col / stepSizeCols)] =
					matchedBlocksSorted[(row - imageBlock.bottom) * imageBlock.width() + col - imageBlock.left].getPatches();
			}
		}

		matchedBlocksSorted.clear();
	}

	void ImageBlockProcessor::computeIntegralImage(const std::vector<double>& pixels, const Rectangle& imageBlock,
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
			long double s = pixels[row * imageBlock.width()];
			integralImage[row * imageBlock.width()] = integralImage[(row - 1) * imageBlock.width()] + s;
			for (int col = 1; col < imageBlock.width(); ++col)
			{
				s += (double)pixels[row * imageBlock.width() + col];
				integralImage[row * imageBlock.width() + col] = integralImage[(row - 1) * imageBlock.width() + col] + s;
			}
		}
	}

	void ImageBlockProcessor::computeNMostSimilarNaive(std::vector<IDX2>& matchedBlocks, const IDX2& position, const ImagePatch& templatePatch,
		size_t windowSizeRows, size_t windowSizeCols,
		size_t maxSimilar, float maxDistance, int norm)
	{
		//do block matching
		int halfWindowSizeRows = windowSizeRows / 2;
		int halfWindowSizeCols = windowSizeCols / 2;

		ImagePatch refPatch;
		refPatch.col = position.col;
		refPatch.row = position.row;
		refPatch.height = 6;
		refPatch.width = 6;
		SortedPatchCollection foundMatches;

		for (int shiftRows = -halfWindowSizeRows; shiftRows <= halfWindowSizeRows; ++shiftRows)
		{
			for (int shiftCols = -halfWindowSizeCols; shiftCols <= halfWindowSizeCols; ++shiftCols)
			{
				ImagePatch currentPatch;
				currentPatch.col = position.col + shiftCols;
				currentPatch.row = position.row + shiftRows;
				currentPatch.height = 6;
				currentPatch.width = 6;
				float distance = m_image.blockMatch_Naive(refPatch, currentPatch, 0, 2);

				if (distance < maxDistance)
				{
					foundMatches.insertPatch32(IDX2(currentPatch.row, currentPatch.col, distance));
					//matchedBlocks.push_back(IDX2(currentPatch.row, currentPatch.col, distance));
				}
			}
		}

		//std::sort(matchedBlocks.begin(), matchedBlocks.end());
		//if (matchedBlocks.size() > 32)
		//{
		//	matchedBlocks.erase(matchedBlocks.begin() + 32, matchedBlocks.end());
		//}
		matchedBlocks = foundMatches.getPatches();
	}

}