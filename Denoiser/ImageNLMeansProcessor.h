#pragma once

#include "Image.h"
#include "NLMeansSettings.h"
#include "IDX2.h"
#include "BufferAggregator.h"
#include "common.h"

namespace Denoise
{

	class ImageNLMeansProcessor
	{
	public:
		ImageNLMeansProcessor(Image* image, Image* imageResult);
		~ImageNLMeansProcessor();

		void process(const NLMeansSettings& settings, bool processMatching = true);

	private:
		Image* m_image;
		Image* m_imageResult;
		NLMeansSettings m_settings;
		BufferAggregator m_buffer;
		std::vector<std::vector<IDX2> > m_matchedBlocks;
		bool m_blockMatchingProcessed;
	};

}
