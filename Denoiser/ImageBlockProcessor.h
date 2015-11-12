#pragma once

#include<vector>
#include<string>
#include<iostream>

#include "Image.h"
#include "ImagePatch.h"
#include "IDX2.h"
#include "Rectangle.h"
#include "common.h"

#include "ImageBlockProcessorSettings.h"
#include "BM3DSettings.h"

namespace Denoise
{
	class ImageBlockProcessor
	{
	public:
		ImageBlockProcessor(Image& image);
		~ImageBlockProcessor();

		void computeNMostSimilar(const ImageBlockProcessorSettings& settings,
			std::vector<std::vector<IDX2> >& matchedBlocks);

		void computeNMostSimilar_PARALLEL(const ImageBlockProcessorSettings& settings,
			const BM3DSettings& bm3dSettings,
			std::vector<std::vector<IDX2> >& matchedBlocks);

		void computeNMostSimilarNaive(std::vector<IDX2>& matchedBlocks, const IDX2& position, const ImagePatch& templatePatch,
			index_t windowSizeRows, index_t windowSizeCols,
			index_t maxSimilar, float maxDistance, int norm);

	private:
		Image& m_image;
	};
}
