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
	class BM3DImageBlockProcessorNN
	{
	public:
		BM3DImageBlockProcessorNN(Image* image, Image* imageBasic, Image* imageResult, Image* imageClean);
		~BM3DImageBlockProcessorNN();

		void process(const BM3DSettings& settings,
			bool loadBlocks, float* blocksNoisy, float* blocksReference,
			size_t& numBlocks,
			size_t numBlocks2Save,
			bool processMatching = true);

		friend void bm3dCollaborativeKernel(BM3DImageBlockProcessorNN* processor, index_t threadIdx, const Rectangle& region);
		friend void bm3dWienerKernel(BM3DImageBlockProcessorNN* processor, index_t threadIdx, const Rectangle& region);

	private:

		void processCollaborativeFilter(bool loadBlocks,
			float* blocksNoisy, float* blocksReference,
			size_t& numBlocks,
			size_t numBlocks2Save);
		void processWienerFilter();

		void processBlockMatching(Image* image, bool collaborative);

		Image* m_image;
		Image* m_imageBasic;
		Image* m_imageResult;
		Image* m_imageClean;
		BM3DSettings m_settings;
		BufferAggregator m_buffer;
		std::vector<std::vector<IDX2> > m_matchedBlocks;
		bool m_blockMatchingProcessed;

		ImagePatch m_patchTemplate;

		MUTEX_TYPE m_mutex;
	};
}


