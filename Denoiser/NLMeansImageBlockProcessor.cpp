#include "NLMeansImageBlockProcessor.h"

#include <algorithm>
#include <numeric>

#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/bind.hpp>

#include "Rectangle.h"
#include "ImageBlockProcessor.h"

namespace Denoise
{

	NLMeansImageBlockProcessor::NLMeansImageBlockProcessor(Image* image, Image* imageResult)
		: m_image(image), m_imageResult(imageResult),
		m_buffer(image->fullDimension(), image->numChannels())
	{
	}


	NLMeansImageBlockProcessor::~NLMeansImageBlockProcessor()
	{
	}

	void NLMeansImageBlockProcessor::process(const NLMeansSettings& settings, bool processMatching)
	{
		m_settings = settings;

		for (size_t i = 0; i < m_settings.stdDeviation.size(); ++i)
		{
			std::cout << m_settings.stdDeviation[i] << std::endl;
		}


		//1. Block Matching
		m_patchTemplate = ImagePatch(0, 0, m_settings.patchSize, m_settings.patchSize);

		processBlockMatching(m_image);

		processNLMeans();

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

		m_imageResult->checkImageIntegrity(true);
		m_imageResult->clamp(m_image->minPixelValue(), m_image->maxPixelValue());
	}

	void NLMeansImageBlockProcessor::processNLMeans()
	{
		/*if (m_settings.numThreadsDenoising == 1)
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
		}*/
	}

	void NLMeansImageBlockProcessor::processBlockMatching(Image* image)
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
			blockMatchSettings.maxSimilar = m_settings.MaxNumPatches;

			blockMatchSettings.maxDistance = m_settings.templateMatchingMaxAllowedPatchDistance;
			blockMatchSettings.norm = m_settings.templateMatchingNorm;
			blockMatchSettings.numChannelsToUse = m_settings.templateMatchingNumChannels;
			blockMatchSettings.matchedBlocksAlreadyComputed = 0;
			blockMatchSettings.numThreadsIntegralImageComputation = m_settings.numThreadsBlockMatching;
			blockMatchSettings.numThreadsBlockMatching = m_settings.numThreadsBlockMatching;

			processor.computeNMostSimilar(blockMatchSettings, m_matchedBlocks);
		}

}
