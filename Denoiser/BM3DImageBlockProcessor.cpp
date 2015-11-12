#include "BM3DImageBlockProcessor.h"
#include "ImageBlockProcessor.h"
#include "ImageBlockProcessorSettings.h"
#include "BufferAggregator.h"
#include "BM3DCollaborativeFilterKernel.h"
#include "BM3DWienerFilterKernel.h"
#include "BM3DImageBlockProcessorFunctions.h"
#include "BM3DCollaborativeFunctor.h"
#include "BM3DWienerFunctor.h"
#include "BM3DImageBlockProcessorBOOST.h"

#include "ImagePartitioner.h"
#include "RangePartitioner.h"
#include "ImagePatch.h"
#include "Rectangle.h"

#include "common.h"

#include "Statistics.h"

#include <algorithm>
#include <numeric>

#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/bind.hpp>

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

		processor.computeNMostSimilar_PARALLEL(blockMatchSettings, m_settings, m_matchedBlocks);
	}

	void BM3DImageBlockProcessor::process(const BM3DSettings& settings, bool processMatching)
	{
		//m_image->toColourSpace(Image::OPP);


		//lets remember last settings for future reference
		m_settings = settings;

		for (size_t i = 0; i < m_settings.stdDeviation.size(); ++i)
		{
			std::cout << m_settings.stdDeviation[i] << std::endl;
		}


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

		std::cout << "Collaborative Done! " << std::endl;

		m_imageBasic->checkImageIntegrity(true);

		std::cout << "First Check..." << std::endl;

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

		m_imageBasic->checkImageIntegrity(true);

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

		//Mark the results as being in the same colour space as the orginal
		//m_imageBasic->setColourSpace(m_image->colourSpace());
		//m_imageResult->setColourSpace(m_image->colourSpace());

		//Transform if necessary
		//m_image->toColourSpace(Image::RGB);
		//m_imageBasic->toColourSpace(Image::RGB);
		//m_imageResult->toColourSpace(Image::RGB);

		std::cout << "Done!" << std::endl;
	}

	void BM3DImageBlockProcessor::processCollaborativeFilter()
	{
		if (m_settings.numThreadsDenoising == 1)
		{
			BM3DCollaborativeFunctor kernel(m_image,
				m_buffer, m_settings, m_matchedBlocks, m_patchTemplate,
				m_mutex);
			processCollaborative(kernel, std::pair<size_t, size_t>(0, m_matchedBlocks.size()));
		}
		else
		{
			boost::thread_group denoiseThreads;
			RangePartitioner partitioner;

			partitioner.createPartition(m_matchedBlocks.size(), m_settings.numThreadsDenoising);

			std::vector <BM3DCollaborativeFunctor*> kernels;

			for (index_t t = 0; t < partitioner.numSegments(); ++t)
			{
				kernels.push_back(new BM3DCollaborativeFunctor(m_image,
					m_buffer, m_settings, m_matchedBlocks, m_patchTemplate,
					m_mutex));
			}

			for (index_t t = 0; t < partitioner.numSegments(); ++t)
			{
				denoiseThreads.create_thread(boost::bind(processCollaborative, boost::ref(*kernels[t]), partitioner.getSegment(t)));
			}

			denoiseThreads.join_all();
		}
	}

	void BM3DImageBlockProcessor::processWienerFilter()
	{
		//clear buffer
		m_buffer.clear();

		processBlockMatching(m_imageBasic, false);

		if (m_settings.numThreadsDenoising == 1)
		{
			BM3DWienerFunctor kernel(m_image, m_imageBasic,
				m_buffer, m_settings, m_matchedBlocks, m_patchTemplate, m_mutex);

			processWiener(kernel, std::pair<size_t, size_t>(0, m_matchedBlocks.size()));
		}
		else
		{
			boost::thread_group denoiseThreads;
			RangePartitioner partitioner;

			partitioner.createPartition(m_matchedBlocks.size(), m_settings.numThreadsDenoising);

			std::vector <BM3DWienerFunctor*> kernels;

			for (index_t t = 0; t < partitioner.numSegments(); ++t)
			{
				kernels.push_back(new BM3DWienerFunctor(m_image, m_imageBasic,
					m_buffer, m_settings, m_matchedBlocks, m_patchTemplate, m_mutex));
			}

			for (index_t t = 0; t < partitioner.numSegments(); ++t)
			{
				denoiseThreads.create_thread(boost::bind(processWiener, boost::ref(*kernels[t]), partitioner.getSegment(t)));
			}

			denoiseThreads.join_all();
		}
	}
}

