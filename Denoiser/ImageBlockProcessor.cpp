#include "ImageBlockProcessor.h"

#include <algorithm>
#include <utility>
#include <numeric>

#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/bind.hpp>

#include <tbb\tick_count.h>
#include <tbb\parallel_for.h>
#include <tbb\blocked_range.h>
#include <tbb\task_scheduler_init.h>

#include "SortedPatchCollection.h"

#include "ImageBlockProcessorSettingsInternal.h"
#include "ImageBlockProcessorFunctions.h"
#include "ImagePartitioner.h"
#include "BM3DSettings.h"
#include "IntergralImageComputerTBB.h"
#include "BlockMatchingComputerTBB.h"

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
		//make sure we are accessing the full image
		m_image.accessFullImage();

		std::vector<SortedPatchCollection> matchedBlocksSorted;

		//we only allocate as many blocks as we need
		for (index_t i = 0; i < (settings.imageBlock.width() / settings.stepSizeCols + 1) * (settings.imageBlock.height() / settings.stepSizeRows + 1) + 1; ++i)
		{
			matchedBlocksSorted.push_back(SortedPatchCollection(settings.maxSimilar));
		}

		ImageBlockProcessorSettingsInternal internalSettings;

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
		std::vector<std::vector<float> > distanceImage(settings.numChannelsToUse);
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

	void ImageBlockProcessor::computeNMostSimilar_PARALLEL(const ImageBlockProcessorSettings& settings,
		const BM3DSettings& bm3dSettings,
		std::vector<std::vector<IDX2> >& matchedBlocks)
	{
		//make sure we are accessing the full image
		m_image.accessFullImage();

		std::vector<SortedPatchCollection> matchedBlocksSorted;
		matchedBlocksSorted.reserve((settings.imageBlock.width() / settings.stepSizeCols + 1) * (settings.imageBlock.height() / settings.stepSizeRows + 1) + 1);

		//we only allocate as many blocks as we need
		for (index_t i = 0; i < (settings.imageBlock.width() / settings.stepSizeCols + 1) * (settings.imageBlock.height() / settings.stepSizeRows + 1) + 1; ++i)
		{
			matchedBlocksSorted.push_back(SortedPatchCollection(settings.maxSimilar));
		}

		ImageBlockProcessorSettingsInternal internalSettings;

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
		std::vector<std::vector<std::vector<float> > > distanceImage((settings.windowSizeCols + 1) * 3);
		std::vector<std::vector<std::vector<double> > > integralImage((settings.windowSizeCols + 1) * 3);

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
				temp.first = shiftRows;
				temp.second = shiftCols;
				shifts.push_back(temp);
			}

			for (index_t z = 0; z < 2; ++z)
			{
				if (shiftRows > halfWindowSizeRows)
				{
					break;
				}

				++shiftRows;
				for (int shiftCols = -halfWindowSizeCols; shiftCols <= halfWindowSizeCols; ++shiftCols)
				{
					std::pair<int, int> temp;
					temp.first = shiftRows;
					temp.second = shiftCols;
					shifts.push_back(temp);
				}
			}

			std::cout << "Computed Shifts..." << std::endl;

			//Now compute which shifts to allocate to which thread
			std::vector<std::vector<std::pair<int, int> > > shiftsForThreads;
			std::vector<index_t> shiftsStartIdxs;
			index_t stepSize = shifts.size() / settings.numThreadsIntegralImageComputation;
			index_t counter = 0;
			for (std::vector<std::pair<int, int> >::iterator it = shifts.begin(); it != shifts.end(); it += stepSize)
			{
				shiftsStartIdxs.push_back(counter);

				if (counter + stepSize + 10 >= shifts.size())
				{
					std::vector<std::pair<int, int> > temp(it, shifts.end());
					shiftsForThreads.push_back(temp);
					break;
				}
				else
				{
					std::vector<std::pair<int, int> > temp(it, it + stepSize);
					shiftsForThreads.push_back(temp);
				}

				counter += stepSize;
			}

			std::cout << "Computed Start Idxs" << std::endl;

			//Compute integral images in parallel
			boost::thread_group integralImageThreads;
			for (index_t i = 0; i < shiftsForThreads.size(); ++i)
			{
				std::cout << "Thread: " << i << ": " << shiftsStartIdxs[i] << std::endl;
				integralImageThreads.create_thread(boost::bind(computeIntegralImageForSpecificShiftsBlock,
					boost::ref(m_image),
					boost::ref(distanceImage),
					boost::ref(integralImage),
					boost::ref(shiftsForThreads[i]),
					shiftsStartIdxs[i],
					boost::ref(settings),
					boost::ref(internalSettings)));
			}

			integralImageThreads.join_all();

			std::cout << "Computed Integral Images..." << std::endl;

			ImagePartitioner partitioner(&m_image, bm3dSettings);
			index_t totalNumBlocks;
			partitioner.createPartitionScanlines(settings.numThreadsBlockMatching, totalNumBlocks);
			std::cout << partitioner.numSegments() << std::endl;;

			boost::thread_group blockMatchImageThreads;
			for (index_t i = 0; i < partitioner.numSegments(); ++i)
			{
				std::cout << "Creating BM Thread..." << std::endl;
				ImageBlockProcessorSettings localSettings = settings;
				localSettings.imageBlock = partitioner.getSegment(i);

				ImageBlockProcessorSettingsInternal localInternalSettings = internalSettings;
				if (i == partitioner.numSegments() - 1)
				{
					localInternalSettings.iterateAtBorders = true;
				}
				else
				{
					localInternalSettings.iterateAtBorders = false;
				}

				blockMatchImageThreads.create_thread(boost::bind(computeBlockMatchingForSpecificShifts_doNotComputeIntegralImageBlock,
					boost::ref(m_image),
					boost::ref(matchedBlocksSorted),
					boost::ref(distanceImage),
					boost::ref(integralImage),
					boost::ref(shifts),
					0,
					localSettings,
					localInternalSettings));
			}
			blockMatchImageThreads.join_all();

			std::cout << "Computed Patch Match for Shifts" << std::endl;
			std::cout << "-------------------" << std::endl;
		}

		std::cout << "Matched Blocks Size: " << matchedBlocks.size() << std::endl;
		std::cout << "Matched Blocks Sorted Size: " << matchedBlocksSorted.size() << std::endl;
		for (index_t i = 0; i < matchedBlocks.size(); ++i)
		{
			matchedBlocks[settings.matchedBlocksAlreadyComputed + i] = matchedBlocksSorted[i].getPatches();
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

	void ImageBlockProcessor::computeNMostSimilar_PARALLEL_TBB(const ImageBlockProcessorSettings& settings,
		const BM3DSettings& bm3dSettings,
		std::vector<std::vector<IDX2> >& matchedBlocks)
	{
		//make sure we are accessing the full image
		m_image.accessFullImage();

		std::vector<SortedPatchCollection> matchedBlocksSorted;
		matchedBlocksSorted.reserve((settings.imageBlock.width() / settings.stepSizeCols + 1) * (settings.imageBlock.height() / settings.stepSizeRows + 1) + 1);

		//we only allocate as many blocks as we need
		for (index_t i = 0; i < (settings.imageBlock.width() / settings.stepSizeCols + 1) * (settings.imageBlock.height() / settings.stepSizeRows + 1) + 1; ++i)
		{
			matchedBlocksSorted.push_back(SortedPatchCollection(settings.maxSimilar));
		}

		ImageBlockProcessorSettingsInternal internalSettings;

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
		std::vector<std::vector<std::vector<float> > > distanceImage((settings.windowSizeCols + 1) * 3);
		std::vector<std::vector<std::vector<double> > > integralImage((settings.windowSizeCols + 1) * 3);

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

		tbb::task_scheduler_init init(settings.numThreadsIntegralImageComputation);

		for (int shiftRows = -halfWindowSizeRows; shiftRows <= halfWindowSizeRows; ++shiftRows)
		{

			std::vector<std::pair<int, int> > shifts;
			for (int shiftCols = -halfWindowSizeCols; shiftCols <= halfWindowSizeCols; ++shiftCols)
			{
				std::pair<int, int> temp;
				temp.first = shiftRows;
				temp.second = shiftCols;
				shifts.push_back(temp);
			}

			for (index_t z = 0; z < 2; ++z)
			{
				if (shiftRows > halfWindowSizeRows)
				{
					break;
				}

				++shiftRows;
				for (int shiftCols = -halfWindowSizeCols; shiftCols <= halfWindowSizeCols; ++shiftCols)
				{
					std::pair<int, int> temp;
					temp.first = shiftRows;
					temp.second = shiftCols;
					shifts.push_back(temp);
				}
			}

			IntergralImageComputerTBB integralImageFunctor(settings, internalSettings, shifts, distanceImage, integralImage, m_image);

			tbb::parallel_for<tbb::blocked_range<index_t> >(tbb::blocked_range<index_t>((index_t)0, (index_t)shifts.size()),
				integralImageFunctor);

			ImagePartitioner partitioner(&m_image, bm3dSettings);
			index_t totalNumBlocks;
			partitioner.createPartitionScanlines(settings.numThreadsBlockMatching, totalNumBlocks);

			BlockMatchingComputerTBB blockMatchingFunctor(settings, internalSettings, shifts, distanceImage, integralImage, m_image,
				matchedBlocksSorted, partitioner.getSegments());

			tbb::parallel_for<tbb::blocked_range<index_t> >(tbb::blocked_range<index_t>((index_t)0, (index_t)partitioner.numSegments()),
				blockMatchingFunctor);
		}
		
		for (index_t i = 0; i < matchedBlocks.size(); ++i)
		{
			matchedBlocks[settings.matchedBlocksAlreadyComputed + i] = matchedBlocksSorted[i].getPatches();
		}

		std::cout << "Finished BM" << std::endl;
	}

}