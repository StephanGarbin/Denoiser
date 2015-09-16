#pragma once

#include<vector>
#include<string>
#include<iostream>

#include "Image.h"
#include "ImagePatch.h"
#include "IDX2.h"
#include "Rectangle.h"

namespace Denoise
{
	class ImageBlockProcessor
	{
	public:
		ImageBlockProcessor(Image& image);
		~ImageBlockProcessor();

		void computeNMostSimilar(const ImagePatch& templatePatch, const Rectangle& imageBlock,
			size_t stepSizeRows, size_t stepSizeCols,
			size_t windowSizeRows, size_t windowSizeCols,
			size_t maxSimilar, float maxDistance,
			int norm,
			std::vector<std::vector<IDX2> >& matchedBlocks);

		void computeNMostSimilarNaive(std::vector<IDX2>& matchedBlocks, const IDX2& position, const ImagePatch& templatePatch,
			size_t windowSizeRows, size_t windowSizeCols,
			size_t maxSimilar, float maxDistance, int norm);

	private:
		Image& m_image;

		float patchDistanceIntegralImage(const std::vector<double>& integralImage, const ImagePatch& templatePatch,
			const Rectangle& imageBlock, const IDX2& position);

		void computeIntegralImage(const std::vector<double>& pixels, const Rectangle& imageBlock,
			std::vector<double>& integralImage);
	};
}
