#include "ImageBlockProcessor.h"

#include <algorithm>
#include <utility>
#include <numeric>

#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/bind.hpp>

#include "SortedPatchCollection.h"

#include "ImageBlockProcessorSettingsInternal.h"
#include "ImageBlockProcessorFunctions.h"
#include "ImagePartitioner.h"
#include "RangePartitioner.h"
#include "BM3DSettings.h"
#include "IntergralImageComputerTBB.h"
#include "BlockMatchingComputerTBB.h"

#include "ImageBlockProcessorBOOST.h"

namespace Denoise
{

	ImageBlockProcessor::ImageBlockProcessor(Image& image) : m_image(image)
	{

	}


	ImageBlockProcessor::~ImageBlockProcessor()
	{

	}


	void ImageBlockProcessor::computeNMostSimilar(const ImageBlockProcessorSettings& settings,
		std::vector<std::vector<IDX2> >& matchedBlocks)
	{
		ImageBlockProcessorSettingsInternal internalSettings;
		//make sure we are accessing the full image
		m_image.accessFullImage();

		std::vector<SortedPatchCollection> matchedBlocksSorted;

		internalSettings.blockWidth = (int)std::ceil((float)settings.imageBlock.width() / (float)settings.stepSizeCols) + 1;
		internalSettings.blockHeight = (int)std::ceil((float)settings.imageBlock.height() / (float)settings.stepSizeRows) + 1;

		//we only allocate as many blocks as we need
		for (index_t i = 0; i < internalSettings.blockWidth * internalSettings.blockHeight; ++i)
		{
			matchedBlocksSorted.push_back(SortedPatchCollection(settings.maxSimilar));
		}



		//do block matching
		int halfWindowSizeRows = settings.windowSizeRows / 2;
		int halfWindowSizeCols = settings.windowSizeCols / 2;

		//The block we can scan (but not actually compute patches for)
		internalSettings.accessibleImageBlock.left = std::max<int>(0, (int)settings.imageBlock.left - halfWindowSizeCols - 1);
		internalSettings.accessibleImageBlock.right = std::min<int>((int)m_image.width(), settings.imageBlock.right + halfWindowSizeCols + 1);
		internalSettings.accessibleImageBlock.bottom = std::max<int>(0, (int)settings.imageBlock.bottom - halfWindowSizeRows - 1);
		internalSettings.accessibleImageBlock.top = std::min<int>((int)m_image.height(), settings.imageBlock.top + halfWindowSizeRows + 1);
		internalSettings.offsetCols = settings.imageBlock.left - internalSettings.accessibleImageBlock.left;
		internalSettings.offsetRows = settings.imageBlock.bottom - internalSettings.accessibleImageBlock.bottom;

		//Zero-shifted version of the accessible image block for integral image construction
		internalSettings.accessibleImageBlockShifted.bottom = 0;
		internalSettings.accessibleImageBlockShifted.top = internalSettings.accessibleImageBlock.top - internalSettings.accessibleImageBlock.bottom;
		internalSettings.accessibleImageBlockShifted.left = 0;
		internalSettings.accessibleImageBlockShifted.right = internalSettings.accessibleImageBlock.right - internalSettings.accessibleImageBlock.left;

		//printf("Offset Cols: %d; Offset Rows: %d \n Region Rows: %d - %d \n Region Cols: %d - %d \n Search Rows: %d - %d \n Search Cols: %d - %d \n",
		//	offsetCols, offsetRows, imageBlock.bottom, imageBlock.top, imageBlock.left, imageBlock.right,
		//	accessibleImageBlock.bottom, accessibleImageBlock.top, accessibleImageBlock.left, accessibleImageBlock.right);

		//Compute in double precision to avoid round-off errors for large images
		std::vector<std::vector<double> > distanceImage(settings.numChannelsToUse);
		std::vector<std::vector<double> > integralImage(settings.numChannelsToUse);

		for (index_t c = 0; c < settings.numChannelsToUse; ++c)
		{
			distanceImage[c].resize(internalSettings.accessibleImageBlock.size());
			integralImage[c].resize(internalSettings.accessibleImageBlock.size());
		}

		for (int shiftRows = -halfWindowSizeRows; shiftRows <= halfWindowSizeRows; ++shiftRows)
		{
			for (int shiftCols = -halfWindowSizeCols; shiftCols <= halfWindowSizeCols; ++shiftCols)
			{
				internalSettings.shiftRows = shiftRows;
				internalSettings.shiftCols = shiftCols;
				computeBlockMatchingForSpecificShifts(m_image, matchedBlocksSorted, distanceImage, integralImage, settings, internalSettings);
			}
		}

		distanceImage.clear();
		integralImage.clear();

		std::cout << "Sorted: " << matchedBlocksSorted.size() << std::endl;
		std::cout << "Matched: " << matchedBlocks.size() << std::endl;
		for (index_t i = 0; i < matchedBlocksSorted.size(); ++i)
		{
			matchedBlocks[settings.matchedBlocksAlreadyComputed + i] = matchedBlocksSorted[i].getPatches();
		}

		matchedBlocksSorted.clear();
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

	void ImageBlockProcessor::computeNMostSimilar_PARALLEL(const ImageBlockProcessorSettings& settings,
		const BM3DSettings& bm3dSettings,
		std::vector<std::vector<IDX2> >& matchedBlocks)
	{
		//make sure we are accessing the full image
		m_image.accessFullImage();

		ImageBlockProcessorSettingsInternal internalSettings;
		internalSettings.blockWidth = (int)std::ceil((float)settings.imageBlock.width() / (float)settings.stepSizeCols) + 1;
		internalSettings.blockHeight = (int)std::ceil((float)settings.imageBlock.height() / (float)settings.stepSizeRows) + 1;

		std::vector<SortedPatchCollection> matchedBlocksSorted;

		//we only allocate as many blocks as we need
		for (index_t i = 0; i < internalSettings.blockWidth * internalSettings.blockHeight; ++i)
		{
			matchedBlocksSorted.push_back(SortedPatchCollection(settings.maxSimilar));
		}

		//do block matching
		int halfWindowSizeRows = settings.windowSizeRows / 2;
		int halfWindowSizeCols = settings.windowSizeCols / 2;

		//The block we can scan (but not actually compute patches for)
		internalSettings.accessibleImageBlock.left = std::max<int>(0, (int)settings.imageBlock.left - halfWindowSizeCols - 1);
		internalSettings.accessibleImageBlock.right = std::min<int>((int)m_image.width(), settings.imageBlock.right + halfWindowSizeCols + 1);
		internalSettings.accessibleImageBlock.bottom = std::max<int>(0, (int)settings.imageBlock.bottom - halfWindowSizeRows - 1);
		internalSettings.accessibleImageBlock.top = std::min<int>((int)m_image.height(), settings.imageBlock.top + halfWindowSizeRows + 1);
		internalSettings.offsetCols = settings.imageBlock.left - internalSettings.accessibleImageBlock.left;
		internalSettings.offsetRows = settings.imageBlock.bottom - internalSettings.accessibleImageBlock.bottom;

		//Zero-shifted version of the accessible image block for integral image construction
		internalSettings.accessibleImageBlockShifted.bottom = 0;
		internalSettings.accessibleImageBlockShifted.top = internalSettings.accessibleImageBlock.top - internalSettings.accessibleImageBlock.bottom;
		internalSettings.accessibleImageBlockShifted.left = 0;
		internalSettings.accessibleImageBlockShifted.right = internalSettings.accessibleImageBlock.right - internalSettings.accessibleImageBlock.left;

		//First determine all necessary shifts
		//Compute in double precision to avoid round-off errors for large images
		std::vector<std::vector<std::vector<double> > > distanceImage((settings.windowSizeCols + 1) * 1);
		std::vector<std::vector<std::vector<double> > > integralImage((settings.windowSizeCols + 1) * 1);

		for (index_t i = 0; i < distanceImage.size(); ++i)
		{
			distanceImage[i].resize(settings.numChannelsToUse);
			integralImage[i].resize(settings.numChannelsToUse);
			for (index_t c = 0; c < settings.numChannelsToUse; ++c)
			{
				distanceImage[i][c].resize(internalSettings.accessibleImageBlock.size());
				integralImage[i][c].resize(internalSettings.accessibleImageBlock.size());
			}
		}

		for (int shiftRows = -halfWindowSizeRows; shiftRows <= halfWindowSizeRows; ++shiftRows)
		{

			std::vector<std::pair<int, int> > shifts;
			for (int shiftCols = -halfWindowSizeCols; shiftCols <= halfWindowSizeCols; ++shiftCols)
			{
				std::pair<int, int> temp;
				temp.first = shiftCols;
				temp.second = shiftRows;
				shifts.push_back(temp);
			}

			RangePartitioner integralImageRange;
			integralImageRange.createPartition(shifts.size(), settings.numThreadsIntegralImageComputation);

			boost::thread_group integralImageTG;

			for (size_t t = 0; t < integralImageRange.numSegments(); ++t)
			{
				integralImageTG.create_thread(boost::bind(computeIntegralImages,
					IntergralImageComputerTBB(settings, internalSettings, shifts, distanceImage, integralImage, m_image),
					integralImageRange.getSegment(t)));
			}

			integralImageTG.join_all();

			ImagePartitioner partitioner(&m_image, bm3dSettings);
			index_t totalNumBlocks;
			partitioner.createPartitionScanlines(settings.numThreadsBlockMatching, totalNumBlocks);

			RangePartitioner blockMatchingRange;
			blockMatchingRange.createPartition(partitioner.numSegments(), partitioner.numSegments());

			boost::thread_group blockMatchingTG;

			for (size_t t = 0; t < partitioner.numSegments(); ++t)
			{
				/*computeBlockMatching(BlockMatchingComputerTBB(settings, internalSettings, shifts, distanceImage, integralImage, m_image,
					matchedBlocksSorted, partitioner.getSegments()),
					blockMatchingRange.getSegment(t));*/
				blockMatchingTG.create_thread(boost::bind(computeBlockMatching,
					BlockMatchingComputerTBB(settings, internalSettings, shifts, distanceImage, integralImage, m_image,
					matchedBlocksSorted, partitioner.getSegments()),
					blockMatchingRange.getSegment(t)));
			}


			blockMatchingTG.join_all();
		}
		
		for (index_t i = 0; i < matchedBlocks.size(); ++i)
		{
			matchedBlocks[i] = matchedBlocksSorted[i].getPatches();
		}

		std::cout << "Finished BM!" << std::endl;
	}

}