#include "ImageBlockProcessor.h"

#include <algorithm>
#include <tbb\parallel_for.h>
#include <tbb\blocked_range.h>
#include <tbb\blocked_range2d.h>


ImageBlockProcessor::ImageBlockProcessor(Image& image) : m_image(image)
{

}


ImageBlockProcessor::~ImageBlockProcessor()
{

}


void ImageBlockProcessor::computeNMostSimilar(std::vector<std::vector<IDX2> >& matchedBlocks,
const ImagePatch& templatePatch, size_t stepSizeRows, size_t stepSizeCols,
size_t windowSizeRows, size_t windowSizeCols,
size_t maxSimilar, float maxDistance,
int norm)
{
	//make sure we are accessing the full image
	m_image.accessFullImage();

	//resize result array
	matchedBlocks.clear();
	matchedBlocks.resize((m_image.width() / stepSizeCols) * (m_image.height() / stepSizeRows));

	//do block matching
	int halfWindowSizeRows = windowSizeRows / 2;
	int halfWindowSizeCols = windowSizeCols / 2;

	std::vector<float> distanceImage(m_image.size());
	std::vector<double> integralImage(m_image.size());

	for (int shiftRows = -halfWindowSizeRows; shiftRows <= halfWindowSizeRows; ++shiftRows)
	{
		for (int shiftCols = -halfWindowSizeCols; shiftCols <= halfWindowSizeCols; ++shiftCols)
		{
			std::cout << "Computing Shift: " << shiftRows << ", " << shiftCols << std::endl;

			//A. Compute Pixel Differences
			for (int row = 0; row < m_image.height(); ++row)
			{
				for (int col = 0; col < m_image.width(); ++col)
				{
					int idx = row * m_image.width() + col;
					int compareIdx = (row + shiftRows) * m_image.width() + (col + shiftCols);
					if (row + shiftRows < 0
						|| col + shiftCols < 0
						|| row + shiftRows > m_image.height() - 1
						|| col + shiftCols > m_image.width() - 1)
					{
						distanceImage[idx] = 0.0f;
					}
					else
					{
						distanceImage[idx] = std::pow(m_image.getPixel(0, idx) - m_image.getPixel(0, compareIdx), norm);
					}
				}
			}

			//std::cout << "Distance Image: " << std::endl;
			//for (int row = 0; row < m_image.height(); ++row)
			//{
			//	for (int col = 0; col < m_image.width(); ++col)
			//	{
			//		std::cout << distanceImage[row * m_image.width() + col] << " ";
			//	}
			//	std::cout << std::endl;
			//}
			//std::cout << std::endl;
			
			//B. Compute Integral Image
			computeIntegralImage(distanceImage, integralImage);
			//std::cout << "Integral Image: " << std::endl;
			//for (int row = 0; row < m_image.height(); ++row)
			//{
			//	for (int col = 0; col < m_image.width(); ++col)
			//	{
			//		std::cout << integralImage[row * m_image.width() + col] << " ";
			//	}
			//	std::cout << std::endl;
			//}
			//std::cout << std::endl;

			//C. Evaluate Patch Distances
			for (int row = 0; row < m_image.height() - 6; row += stepSizeRows)
			{
				for (int col = 0; col < m_image.width() - 6; col += stepSizeCols)
				{
					int idx = row * m_image.width() + col;
					int compareIdx = (row + shiftRows) * m_image.width() + (col + shiftCols);
					if (compareIdx < 0 || compareIdx > m_image.width() * m_image.height() - 1)
					{
						continue;
					}
					else
					{
						if (row > 0 && + templatePatch.height < m_image.height() - 1
							&& col > 0 && col + templatePatch.width < m_image.width() - 1)
						{
							float distance = patchDistanceIntegralImage(integralImage, templatePatch, IDX2(row, col));

							if (distance < maxDistance)
							{
								matchedBlocks[(row / stepSizeRows) * (m_image.width() / stepSizeCols) + (col / stepSizeCols)].push_back(
									IDX2(row + shiftRows, col + shiftCols, distance));

								//std::cout << "row = " << row << "; col = " << col << "; shiftRow = " << shiftRows << "; shiftCols = " << shiftCols << "; Distance = " << distance << std::endl;
							}
						}
					}
				}
			}

		}
	}


	//sort & truncate results
	for (int i = 0; i < matchedBlocks.size(); ++i)
	{
		std::sort(matchedBlocks[i].begin(), matchedBlocks[i].end());

		if (matchedBlocks[i].size() > maxSimilar)
		{
			matchedBlocks[i].erase(matchedBlocks[i].begin() + maxSimilar, matchedBlocks[i].end());
		}
	}
}

void ImageBlockProcessor::computeIntegralImage(const std::vector<float>& pixels, std::vector<double>& integralImage)
{
	//see e.g. http://www.ipol.im/pub/art/2014/57/article.pdf for details on this recurrence-relation
	integralImage[0] = pixels[0];

	for (int col = 1; col < m_image.width(); ++col)
	{
		integralImage[col] = integralImage[col - 1] + pixels[col];
	}

	for (int row = 1; row < m_image.height(); ++row)
	{
		double s = pixels[row * m_image.width()];
		integralImage[row * m_image.width()] = integralImage[(row - 1) * m_image.width()] + s;
		for (int col = 1; col < m_image.width(); ++col)
		{
			s += (double)pixels[row * m_image.width() + col];
			integralImage[row * m_image.width() + col] = integralImage[(row - 1) * m_image.width() + col] + s;
		}
	}

	//tbb::parallel_for<tbb::blocked_range<size_t>>(tbb::blocked_range<size_t>(0, m_image.height()),
	//	[&](const tbb::blocked_range<size_t> r)
	//{
	//	for (int row = r.begin(); row != r.end(); ++row)
	//	{
	//		for (int col = 0; col < m_image.width(); ++col)
	//		{
	//			double sum = 0.0;
	//			for (int lrow = 0; lrow <= row; ++lrow)
	//			{
	//				for (int lcol = 0; lcol <= col; ++lcol)
	//				{
	//					sum += (double)pixels[lrow * m_image.width() + lcol];
	//				}
	//			}
	//			integralImage[row * m_image.width() + col] = (float)sum;
	//		}
	//	}
	//});

}

float ImageBlockProcessor::patchDistanceIntegralImage(const std::vector<double>& integralImage,
	const ImagePatch& templatePatch, IDX2& position)
{
	float result = 0.0f;
	//L4
	result = integralImage[(position.row + templatePatch.height - 1) * m_image.width()
		+ position.col + templatePatch.width - 1];

	//if (position.col > 0)
	//{
	//	result -= integralImage[(position.row + templatePatch.height - 1) * m_image.width()
	//		+ position.col - 1];
	//}
	//if (position.row > 0)
	//{
	//	result -= integralImage[(position.row - 1) * m_image.width()
	//		+ position.col + templatePatch.width - 1];
	//}

	//if (position.row > 0 && position.col > 0)
	//{
	//	result += integralImage[(position.row - 1) * m_image.width()
	//		+ position.col - 1];
	//}

	result -= integralImage[(position.row + templatePatch.height - 1) * m_image.width()
		+ position.col - 1];

	result -= integralImage[(position.row - 1) * m_image.width()
		+ position.col + templatePatch.width - 1];

	result += integralImage[(position.row - 1) * m_image.width()
		+ position.col - 1];

	return result;
}

void ImageBlockProcessor::computeNMostSimilarNaive(std::vector<IDX2>& matchedBlocks, IDX2& position, const ImagePatch& templatePatch,
	size_t windowSizeRows, size_t windowSizeCols,
	size_t maxSimilar, float maxDistance, int norm)
{
	//do block matching
	int halfWindowSizeRows = windowSizeRows / 2;
	int halfWindowSizeCols = windowSizeCols / 2;

	ImagePatch refPatch;
	refPatch.col = position.col;
	refPatch.row = position.row;
	refPatch.height = 2;
	refPatch.width = 2;

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
