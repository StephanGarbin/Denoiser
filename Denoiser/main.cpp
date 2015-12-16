#include <vector>
#include <string>
#include <iostream>

#include "lodepng.h"

#include "Image.h"
#include "ImageBlockProcessor.h"
#include "IDX2.h"
#include "Rectangle.h"
#include "ImagePatch.h"

#include "BM3DImageBlockProcessor.h"
#include "BM3DSettings.h"

int main(int argc, char* argv[])
{
	float stdDeviation;
	index_t numThreadsBM;
	float adaptiveFactor = 0.0f;
	std::string inputFile;
	std::string outputFile;

	if (argc != 5)
	{
		std::cout << "Error: Please provide standard deviation & numThreads (BM), input & output file!" << std::endl;
		return 0;
	}
	else
	{
		stdDeviation = std::atof(argv[1]);
		numThreadsBM = std::atoi(argv[2]);
		inputFile = std::string(argv[3]);
		outputFile = std::string(argv[4]);
	}

	
	Denoise::Image* image = new Denoise::Image();

	image->readFromFile(inputFile);

	image->checkImageIntegrity(true);
	image->normalise();

	Denoise::Image result(*image);
	Denoise::Image basic(*image);

	std::cout << "Filtering..." << std::endl;

	Denoise::BM3DImageBlockProcessor bm3dFilter(image, &basic, &result);

	Denoise::BM3DSettings bm3dFilterSettings;
	
	std::vector<float> smoothness;
	smoothness.push_back(stdDeviation);
	smoothness.push_back(stdDeviation);
	smoothness.push_back(stdDeviation);

	bm3dFilterSettings.init2defaults(smoothness, false);
	bm3dFilterSettings.limitHardwareConcurrency(numThreadsBM);
	bm3dFilterSettings.enableBlockStatisticalAveraging(0.35f);

	//bm3dFilterSettings.enableMeanAdaptiveThresholding(1.0f, 12.0f);

	bm3dFilter.process(bm3dFilterSettings, true);

	image->undoNormalise();
	result.undoNormalise();
	basic.undoNormalise();

	basic.setAlphaToOne();
	result.setAlphaToOne();

	result.save2File(outputFile);

	delete image;
}
