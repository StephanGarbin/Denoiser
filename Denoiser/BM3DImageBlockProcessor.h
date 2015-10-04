#pragma once

#include "Image.h"
#include "BM3DSettings.h"
#include "IDX2.h"
#include "BufferAggregator.h"
#include "Rectangle.h"

namespace Denoise
{
	class BM3DImageBlockProcessor
	{
	public:
		BM3DImageBlockProcessor(Image* image, Image* imageResult);
		~BM3DImageBlockProcessor();

		void process(const BM3DSettings& settings, bool processMatching = true);

		friend void bm3dCollaborativeKernel(BM3DImageBlockProcessor* processor, size_t threadIdx, const Rectangle& region);
		friend void bm3dWienerKernel(BM3DImageBlockProcessor* processor, size_t threadIdx, const Rectangle& region);

	private:

		void processCollaborativeFilter();
		void processWienerFilter();

		Image* m_image;
		Image* m_imageResult;
		BM3DSettings m_settings;
		BufferAggregator m_buffer;
		std::vector<std::vector<IDX2> > m_matchedBlocks;
		bool m_blockMatchingProcessed;
	};
}

