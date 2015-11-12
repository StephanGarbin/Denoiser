#pragma once

#include "Image.h"
#include "BM3DSettings.h"
#include "IDX2.h"
#include "BufferAggregator.h"
#include "Rectangle.h"
#include "common.h"
#include "Definitions.h"

namespace Denoise
{
	class BM3DImageBlockProcessor
	{
	public:
		BM3DImageBlockProcessor(Image* image, Image* imageBasic, Image* imageResult);
		~BM3DImageBlockProcessor();

		void process(const BM3DSettings& settings, bool processMatching = true);

		friend void bm3dCollaborativeKernel(BM3DImageBlockProcessor* processor, index_t threadIdx, const Rectangle& region);
		friend void bm3dWienerKernel(BM3DImageBlockProcessor* processor, index_t threadIdx, const Rectangle& region);

	private:

		void processCollaborativeFilter();
		void processWienerFilter();

		void processBlockMatching(Image* image, bool collaborative);

		Image* m_image;
		Image* m_imageBasic;
		Image* m_imageResult;
		BM3DSettings m_settings;
		BufferAggregator m_buffer;
		std::vector<std::vector<IDX2> > m_matchedBlocks;
		bool m_blockMatchingProcessed;

		ImagePatch m_patchTemplate;

		MUTEX_TYPE m_mutex;
	};
}

