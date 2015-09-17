#include "ImageBlockProcessor.h"

#include <algorithm>
#include <tbb\parallel_for.h>
#include <tbb\blocked_range.h>
#include <tbb\blocked_range2d.h>
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
		std::vector<std::vector<IDX2> >& matchedBlocks)
	{
		//make sure we are accessing the full image
		m_image.accessFullImage();

		//resize result array
		//matchedBlocks.resize((m_image.width() / stepSizeCols) * (m_image.height() / stepSizeRows));
		//for (size_t i = 0; i < matchedBlocks.size(); ++i)
		//{
		//	matchedBlocks[i].reserve((windowSizeRows * windowSizeCols) / 2);
		//}

		std::vector<SortedPatchCollection> matchedBlocksSorted;
		matchedBlocksSorted.resize(imageBlock.size());

		//do block matching
		int halfWindowSizeRows = windowSizeRows / 2;
		int halfWindowSizeCols = windowSizeCols / 2;

		//Compute in double precision to avoid round-off errors for large images
		std::vector<double> distanceImage(imageBlock.size());
		std::vector<double> integralImage(imageBlock.size());

		for (int shiftRows = -halfWindowSizeRows; shiftRows <= halfWindowSizeRows; ++shiftRows)
		{
			for (int shiftCols = -halfWindowSizeCols; shiftCols <= halfWindowSizeCols; ++shiftCols)
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
							distanceImage[blockIdx] = 0.0f;
						}
						else
						{
							distanceImage[blockIdx] = std::pow((double)m_image.getPixel(0, idx) - (double)m_image.getPixel(0, compareIdx), norm);
						}
					}
				}

				//B. Compute Integral Image
				computeIntegralImage(distanceImage, imageBlock, integralImage);

				//C. Evaluate Patch Distances
				for (int row = imageBlock.bottom; row < imageBlock.top - templatePatch.height; row += stepSizeRows)
				{
					for (int col = imageBlock.left; col < imageBlock.right - templatePatch.width; col += stepSizeCols)
					{
						if (row - imageBlock.bottom > 0 && row - imageBlock.bottom + templatePatch.height < m_image.height() - templatePatch.height - 1
							&& col - imageBlock.left > 0 && col - imageBlock.left + templatePatch.width < m_image.width() - templatePatch.width - 1)
						{
							float distance = patchDistanceIntegralImage(integralImage, templatePatch, imageBlock, IDX2(row - imageBlock.bottom, col - imageBlock.left));

							if (distance < maxDistance)
							{
								/*matchedBlocks[(row / stepSizeRows) * (m_image.width() / stepSizeCols) + (col / stepSizeCols)].push_back(
									IDX2(row + shiftRows, col + shiftCols, distance));*/
								matchedBlocksSorted[(row - imageBlock.bottom) * imageBlock.width() + col - imageBlock.left].insertPatch32(
									IDX2(row + shiftRows, col + shiftCols, distance));
							}
						}
					}
				}
			}
		}

		//sort & truncate results (if necessary)
		//for (int i = 0; i < matchedBlocks.size(); ++i)
		//{
		//	if (matchedBlocks[i].empty())
		//	{
		//		continue;
		//	}

		//	std::sort(matchedBlocks[i].begin(), matchedBlocks[i].end());

		//	if (matchedBlocks[i].size() > maxSimilar)
		//	{
		//		matchedBlocks[i].erase(matchedBlocks[i].begin() + maxSimilar, matchedBlocks[i].end());
		//	}
		//}
		
		matchedBlocks.resize((m_image.width() / stepSizeCols) * (m_image.height() / stepSizeRows));

		std::cout << "Done getting patches; " << matchedBlocksSorted.size() << "; " << matchedBlocks.size() << std::endl;

		for (int row = imageBlock.bottom; row < imageBlock.top - templatePatch.height; row += stepSizeRows)
		{
			for (int col = imageBlock.left; col < imageBlock.right - templatePatch.width; col += stepSizeCols)
			{
				//matchedBlocks[(row / stepSizeRows) * (m_image.width() / stepSizeCols) + (col / stepSizeCols)] =
				//	matchedBlocksSorted[(row - imageBlock.bottom) * imageBlock.width() + col - imageBlock.left].getPatches();
			}
		}

		matchedBlocksSorted.clear();
	}

	void ImageBlockProcessor::computeIntegralImage(const std::vector<double>& pixels, const Rectangle& imageBlock,
		std::vector<double>& integralImage)
	{
		//see e.g. http://www.ipol.im/pub/art/2014/57/article.pdf for details on this recurrence-relation
		integralImage[0] = pixels[0];

		for (int col = 1; col < imageBlock.width(); ++col)
		{
			integralImage[col] = integralImage[col - 1] + pixels[col];
		}

		for (int row = 1; row < imageBlock.height(); ++row)
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

	float ImageBlockProcessor::patchDistanceIntegralImage(const std::vector<double>& integralImage, const ImagePatch& templatePatch,
		const Rectangle& imageBlock, const IDX2& position)
	{
		double result = integralImage[(position.row + templatePatch.height - 1) * imageBlock.width()
			+ position.col + templatePatch.width - 1];

		result -= integralImage[(position.row + templatePatch.height - 1) * imageBlock.width()
			+ position.col - 1];

		result -= integralImage[(position.row - 1) * imageBlock.width()
			+ position.col + templatePatch.width - 1];

		result += integralImage[(position.row - 1) * imageBlock.width()
			+ position.col - 1];

		return (float)result;
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
					matchedBlocks.push_back(IDX2(currentPatch.row, currentPatch.col, distance));
				}
			}
		}

		std::sort(matchedBlocks.begin(), matchedBlocks.end());
		if (matchedBlocks.size() > 32)
		{
			matchedBlocks.erase(matchedBlocks.begin() + 32, matchedBlocks.end());
		}
	}

}