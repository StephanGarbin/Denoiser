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
			std::vector<std::vector<IDX2> >& matchedBlocks,
			size_t numChannelsToUse);

		void computeNMostSimilarNaive(std::vector<IDX2>& matchedBlocks, const IDX2& position, const ImagePatch& templatePatch,
			size_t windowSizeRows, size_t windowSizeCols,
			size_t maxSimilar, float maxDistance, int norm);

	private:
		Image& m_image;

		inline double patchDistanceIntegralImage(const std::vector<double>& integralImage, const ImagePatch& templatePatch,
			const Rectangle& imageBlock, const IDX2& position);

		void computeIntegralImage(const std::vector<double>& pixels, const Rectangle& imageBlock,
			std::vector<double>& integralImage);
	};

	double ImageBlockProcessor::patchDistanceIntegralImage(const std::vector<double>& integralImage, const ImagePatch& templatePatch,
		const Rectangle& imageBlock, const IDX2& position)
	{
		double result = integralImage[(position.row + templatePatch.height - 1) * imageBlock.width()
			+ position.col + templatePatch.width - 1];

		result -= integralImage[(position.row + templatePatch.height - 1) * imageBlock.width()
			+ position.col - 1];

		result -= integralImage[(position.row - 1) * imageBlock.width()
			+ position.col + templatePatch.width - 1];

		result += integralImage[(position.row - 1) * imageBlock.width()
			+ position.col - 1];

		return result;
	}
}
