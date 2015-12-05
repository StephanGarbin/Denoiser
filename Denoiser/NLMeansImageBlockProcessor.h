#pragma once

#include <vector>

#include "Image.h"
#include "BufferAggregator.h"
#include "Definitions.h"
#include "NLMeansSettings.h"

namespace Denoise
{

	class NLMeansImageBlockProcessor
	{
	public:
		NLMeansImageBlockProcessor(Image* image, Image* imageResult);
		~NLMeansImageBlockProcessor();

		void process(const NLMeansSettings& settings, bool processMatching = true);

	private:
		void processNLMeans();

		void processBlockMatching(Image* image);

		Image* m_image;
		Image* m_imageResult;
		NLMeansSettings m_settings;
		BufferAggregator m_buffer;
		std::vector<std::vector<IDX2> > m_matchedBlocks;
		bool m_blockMatchingProcessed;

		ImagePatch m_patchTemplate;

		MUTEX_TYPE m_mutex;
	};

}

