#pragma once

#include<vector>
#include<string>
#include<iostream>

#include "Image.h"
#include "ImagePatch.h"
#include "IDX2.h"

class ImageBlockProcessor
{
public:
	ImageBlockProcessor(Image& image);
	~ImageBlockProcessor();

	void computeNMostSimilar(std::vector<std::vector<IDX2> >& matchedBlocks,
		const ImagePatch& templatePatch, size_t stepSizeRows, size_t stepSizeCols,
		size_t windowSizeRows, size_t windowSizeCols,
		size_t maxSimilar, float maxDistance,
		int norm);

	void computeNMostSimilarNaive(std::vector<IDX2>& matchedBlocks, IDX2& position,  const ImagePatch& templatePatch,
		size_t windowSizeRows, size_t windowSizeCols,
		size_t maxSimilar, float maxDistance, int norm);

private:
	Image& m_image;

	float patchDistanceIntegralImage(const std::vector<double>& integralImage, const ImagePatch& templatePatch, IDX2& position);

	void computeIntegralImage(const std::vector<float>& pixels, std::vector<double>& integralImage);
};
