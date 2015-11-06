#include "BM3DImageBlockProcessor.h"
#include "ImageBlockProcessor.h"
#include "ImageBlockProcessorSettings.h"
#include "BufferAggregator.h"
#include "BM3DCollaborativeFilterKernel.h"
#include "BM3DWienerFilterKernel.h"
#include "BM3DImageBlockProcessorFunctions.h"
#include "BM3DCollaborativeTBB.h"
#include "BM3DWienerTBB.h"

#include "ImagePartitioner.h"
#include "ImagePatch.h"
#include "Rectangle.h"

#include "common.h"

#include "DEBUG_HELPER.h"
#include "Statistics.h"

#include <algorithm>
#include <numeric>

#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/bind.hpp>

#include <tbb\tick_count.h>
#include <tbb\blocked_range.h>
#include <tbb\parallel_for.h>
#include <tbb\task_scheduler_init.h>

namespace Denoise
{

	BM3DImageBlockProcessor::BM3DImageBlockProcessor(Image* image, Image* imageBasic, Image* imageResult)
		: m_image(image), m_imageBasic(imageBasic), m_imageResult(imageResult),
		m_buffer(image->fullDimension(), image->numChannels())
	{
		m_blockMatchingProcessed = false;
	}


	BM3DImageBlockProcessor::~BM3DImageBlockProcessor()
	{

	}

	void BM3DImageBlockProcessor::processBlockMatching(Image* image, bool collaborative)
	{
		m_matchedBlocks.clear();

		tbb::tick_count start = tbb::tick_count::now();

		ImagePatch patchTemplate(0, 0, m_settings.patchSize, m_settings.patchSize);

		Rectangle matchRegion(0, image->width(), image->height(), 0);

		m_matchedBlocks.resize((matchRegion.width() / m_settings.stepSizeCols + 1)
			* (matchRegion.height() / m_settings.stepSizeRows + 1));

		ImageBlockProcessor processor(*m_image);

		ImageBlockProcessorSettings blockMatchSettings;
		blockMatchSettings.templatePatch = patchTemplate;
		blockMatchSettings.imageBlock = matchRegion;
		blockMatchSettings.stepSizeRows = m_settings.stepSizeRows;
		blockMatchSettings.stepSizeCols = m_settings.stepSizeCols;
		blockMatchSettings.windowSizeRows = m_settings.searchWindowSize;
		blockMatchSettings.windowSizeCols = m_settings.searchWindowSize;
		if (collaborative)
		{
			blockMatchSettings.maxSimilar = m_settings.numPatchesPerBlockCollaborative;
		}
		else
		{
			blockMatchSettings.maxSimilar = m_settings.numPatchesPerBlockWiener;
		}
		blockMatchSettings.maxDistance = m_settings.templateMatchingMaxAllowedPatchDistance;
		blockMatchSettings.norm = m_settings.templateMatchingNorm;
		blockMatchSettings.numChannelsToUse = m_settings.templateMatchingNumChannels;
		blockMatchSettings.matchedBlocksAlreadyComputed = 0;
		blockMatchSettings.numThreadsIntegralImageComputation = m_settings.numThreadsBlockMatching;
		blockMatchSettings.numThreadsBlockMatching = m_settings.numThreadsBlockMatching;

		processor.computeNMostSimilar_PARALLEL_TBB(blockMatchSettings, m_settings, m_matchedBlocks);
		/*std::cout << "Computing Sequential Comparison..." << std::endl;
		std::vector<std::vector<IDX2> > matchedBlocks2;
		{
			Rectangle matchRegion(0, image->width(), image->height(), 0);

			ImageBlockProcessor blockProcessor(*image);

			matchedBlocks2.resize((matchRegion.width() / m_settings.stepSizeCols + 1)
				* (matchRegion.height() / m_settings.stepSizeRows + 1));

			ImageBlockProcessorSettings blockMatchSettings;
			blockMatchSettings.templatePatch = patchTemplate;
			blockMatchSettings.imageBlock = matchRegion;
			blockMatchSettings.stepSizeRows = m_settings.stepSizeRows;
			blockMatchSettings.stepSizeCols = m_settings.stepSizeCols;
			blockMatchSettings.windowSizeRows = m_settings.searchWindowSize;
			blockMatchSettings.windowSizeCols = m_settings.searchWindowSize;
			if (collaborative)
			{
				blockMatchSettings.maxSimilar = m_settings.numPatchesPerBlockCollaborative;
			}
			else
			{
				blockMatchSettings.maxSimilar = m_settings.numPatchesPerBlockWiener;
			}
			blockMatchSettings.maxDistance = m_settings.templateMatchingMaxAllowedPatchDistance;
			blockMatchSettings.norm = m_settings.templateMatchingNorm;
			blockMatchSettings.numChannelsToUse = m_settings.templateMatchingNumChannels;
			blockMatchSettings.matchedBlocksAlreadyComputed = 0;

			blockProcessor.computeNMostSimilar(blockMatchSettings, matchedBlocks2);
		}

		std::cout << "Comparing..." << std::endl;
		size_t counter = 0;
		for (index_t i = 0; i < m_matchedBlocks.size(); ++i)
		{
			bool matches = true;
			for (index_t b = 0; b < m_matchedBlocks[i].size(); ++b)
			{
				if (m_matchedBlocks[i][b].col != matchedBlocks2[i][b].col || m_matchedBlocks[i][b].row != matchedBlocks2[i][b].row)
				{
					matches = false;
					break;
				}
			}
			if (!matches)
			{
				++counter;
			}
		}

		std::cout << ((float)counter / (float)m_matchedBlocks.size()) * 100.0f << "% blocks are different" << std::endl;*/

		std::cout << "Finished Block Matching..." << std::endl;
		tbb::tick_count end = tbb::tick_count::now();
		std::cout << "Time: " << (end - start).seconds() << "s." << std::endl;
	}

	void BM3DImageBlockProcessor::process(const BM3DSettings& settings, bool processMatching)
	{
		tbb::task_scheduler_init init(settings.numThreadsBlockMatching);
		//tbb::task_scheduler_init init(1);

		tbb::tick_count start = tbb::tick_count::now();

		//lets remember last settings for future reference
		m_settings = settings;

		//1. Block Matching
		m_patchTemplate = ImagePatch(0, 0, m_settings.patchSize, m_settings.patchSize);

		if (!processMatching)
		{
			if (!m_blockMatchingProcessed)
			{
				std::cout << "BM3D ERROR: Block Matching must be processed at least once before" << std::endl;
				return;
			}
		}
		else
		{
			processBlockMatching(m_image, true);
		}

		processCollaborativeFilter();

		//divide buffers
		m_buffer.divideBuffers();

		//set result image
		for (index_t channel = 0; channel < 3; ++channel)
		{
			for (index_t row = 0; row < m_image->height(); row += 1)
			{
				for (index_t col = 0; col < m_image->width(); col += 1)
				{
					m_imageBasic->setPixel(channel, row, col, m_buffer.getValueResult(channel, row, col));
				}
			}
		}

		//m_imageBasic->clamp(m_image->minPixelValue(), m_image->maxPixelValue());

		if (!m_settings.disableWienerFilter)
		{
			processWienerFilter();

			//divide buffers
			m_buffer.divideBuffers();

			//set result image
			for (index_t channel = 0; channel < 3; ++channel)
			{
				for (index_t row = 0; row < m_image->height(); row += 1)
				{
					for (index_t col = 0; col < m_image->width(); col += 1)
					{
						m_imageResult->setPixel(channel, row, col, m_buffer.getValueResult(channel, row, col));
					}
				}
			}
		}

		//clamp final image to ensure we don't get any illegal values (especially for quantised images)
		m_imageResult->clamp(m_image->minPixelValue(), m_image->maxPixelValue());

		tbb::tick_count end = tbb::tick_count::now();
		std::cout << "BM3D Denoiser ran in " << (end - start).seconds() << std::endl;
	}

	void BM3DImageBlockProcessor::processCollaborativeFilter()
	{
		BM3DCollaborativeTBB collaborativeFunctor(m_image,
			m_buffer, m_settings, m_matchedBlocks, m_patchTemplate,
			m_mutex);

		std::cout << m_matchedBlocks.size() << std::endl;
		tbb::parallel_for<tbb::blocked_range<size_t> >(tbb::blocked_range<size_t>((size_t)0, m_matchedBlocks.size()),
			collaborativeFunctor);
	}

	void BM3DImageBlockProcessor::processWienerFilter()
	{
		//clear buffer
		m_buffer.clear();

		processBlockMatching(m_imageBasic, false);

		BM3DWienerTBB wienerFunctor(m_image, m_imageBasic,
			m_buffer, m_settings, m_matchedBlocks, m_patchTemplate, m_mutex);

		std::cout << m_matchedBlocks.size() << std::endl;
		tbb::parallel_for<tbb::blocked_range<size_t> >(tbb::blocked_range <size_t>((size_t)0, m_matchedBlocks.size()),
			wienerFunctor);
	}
}

