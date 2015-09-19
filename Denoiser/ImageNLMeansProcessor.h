#pragma once

#include "Image.h"
#include "NLMeansSettings.h"
#include "IDX2.h"
#include "BufferAggregator.h"

namespace Denoise
{

	class ImageNLMeansProcessor
	{
	public:
		ImageNLMeansProcessor(Image* image);
		~ImageNLMeansProcessor();

		void process(const NLMeansSettings& settings, bool processMatching = true);

	private:
		Image* m_image;
		NLMeansSettings m_settings;
		BufferAggregator m_buffer;
		std::vector<std::vector<IDX2> > m_matchedBlocks;
		bool m_blockMatchingProcessed;
	};

}
