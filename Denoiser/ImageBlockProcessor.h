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

	private:
		Image& m_image;
	};
}
