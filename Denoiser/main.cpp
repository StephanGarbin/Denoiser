#include <vector>
#include <string>
#include <iostream>

#include "lodepng.h"

#include "Image.h"
#include "ImageBlockProcessor.h"
#include "IDX2.h"
#include "Rectangle.h"
#include "ImagePatch.h"

#include "ImageNLMeansProcessor.h"
#include "NLMeansSettings.h"

#include "BM3DImageBlockProcessor.h"
#include "BM3DSettings.h"

#include <tbb\tick_count.h>

void loadImage(Denoise::Image** image, const std::string& fileName);

void saveImage(Denoise::Image* image, const std::string& fileName);


int main(int argc, char* argv[])
{
	float stdDeviation;
	index_t numThreadsBM;
	float adaptiveFactor = 0.0f;

	if (argc != 3)
	{
		std::cout << "Error: Please provide standard deviation & numThreads (BM)!" << std::endl;
		std::cout << "Value used (stdDeviation): 1" << std::endl;
		stdDeviation = 1.0f;
		std::cout << "Value used (numThreads BM): 1";
		numThreadsBM = 1;
	}
	else
	{
		stdDeviation = std::atof(argv[1]);
		numThreadsBM = std::atoi(argv[2]);
		//numThreadsBM = 1;
		//adaptiveFactor = std::atof(argv[2]);
	}

	//std::string inputFile = "C:/Users/Stephan/Desktop/tiger.png";
	//std::string inputFile = "C:/Users/Stephan/Desktop/llama.png";
	//std::string outputFile = "C:/Users/Stephan/Desktop/llama_padded.png";
	//std::string inputFile = "C:/Users/Stephan/Desktop/noisyTrees2.png";
	//std::string outputFile = "C:/Users/Stephan/Desktop/noisyTreesNew2b.png";

	std::string inputFile = "C:/Users/Stephan/Desktop/noisyTrees.png";
	std::string outputFile = "C:/Users/Stephan/Desktop/noisyTreesNew_smoothness2.png";

	//std::string inputFile = "C:/Users/Stephan/Desktop/computerNoisy.png";
	//std::string outputFile = "C:/Users/Stephan/Desktop/computerNew.png";

	//std::string inputFile = "C:/Users/Stephan/Desktop/RendermanTestScene1.png";
	//std::string outputFile = "C:/Users/Stephan/Desktop/RendermanTestScene1BM3D.png";

	//std::string inputFile = "C:/Users/Stephan/Desktop/tiger_high.png";
	//std::string outputFile = "C:/Users/Stephan/Desktop/tiger_high_denoised.png";

	//std::string inputFile = "C:/Users/Stephan/Desktop/tiger_1K.png";
	//std::string outputFile = "C:/Users/Stephan/Desktop/tiger_1K_denoised_b.png";

	//std::string inputFile = "C:/Users/Stephan/Desktop/RendermanTestScene1.png";
	//std::string outputFile = "C:/Users/Stephan/Desktop/RendermanTestScene1BM3D.png";
	
	Denoise::Image* image = nullptr;

	loadImage(&image, inputFile);
	image->checkImageIntegrity(true);
	image->normalise();

	Denoise::Image result(*image);
	Denoise::Image basic(*image);

	Denoise::BM3DImageBlockProcessor bm3dFilter(image, &basic, &result);

	Denoise::BM3DSettings bm3dFilterSettings;
	bm3dFilterSettings.templateMatchingMaxAllowedPatchDistance = 5.0000001f;
	bm3dFilterSettings.templateMatchingNorm = 3;
	bm3dFilterSettings.templateMatchingNumChannels = 1;

	bm3dFilterSettings.numPatchesPerBlockCollaborative = 16;
	bm3dFilterSettings.patchSize = 8;
	bm3dFilterSettings.searchWindowSize = 32;
	bm3dFilterSettings.stepSizeCols = 3;
	bm3dFilterSettings.stepSizeRows = 3;
	bm3dFilterSettings.usePatchWeighting = false;
	bm3dFilterSettings.stdDeviation = stdDeviation;

	bm3dFilterSettings.averageBlocksBasedOnStdCollaborative = false;
	bm3dFilterSettings.averageBlocksBasedOnStdWiener = false; // turn only this to true if enabling the mode
	bm3dFilterSettings.averageBlocksBasedOnStdFactor = 0.75f;

	bm3dFilterSettings.meanAdaptiveThresholding = false;
	bm3dFilterSettings.meanAdaptiveThresholdingFactor = adaptiveFactor;

	bm3dFilterSettings.numThreadsBlockMatching = numThreadsBM;
	bm3dFilterSettings.numPatchesPerBlockWiener = 32;
	bm3dFilterSettings.disableWienerFilter = false;

	bm3dFilter.process(bm3dFilterSettings, true);

	image->undoNormalise();
	result.undoNormalise();
	basic.undoNormalise();

	basic.setAlphaToOne();
	result.setAlphaToOne();

	saveImage(&result, outputFile);

	delete image;
}

void loadImage(Denoise::Image** image, const std::string& fileName)
{
	std::vector<unsigned char> png;
	std::vector<unsigned char> rawImage; //the raw pixels
	unsigned width, height;

	//load and decode
	lodepng::load_file(png, fileName);
	unsigned error = lodepng::decode(rawImage, width, height, png);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

	Denoise::Dimension dim(width, height);
	*image = new Denoise::Image(dim, Denoise::Image::FLOAT_4);

	for (index_t i = 0; i < rawImage.size(); i+=4)
	{
		for (index_t c = 0; c < 4; ++c)
		{
			(*image)->setPixel(c, i / 4, (float)rawImage[i + c]);
		}
	}
}

void saveImage(Denoise::Image* image, const std::string& fileName)
{
	std::vector<unsigned char> png;

	std::vector<unsigned char> rawImage;
	rawImage.resize(image->width() * image->height() * 4);

	for (index_t row = 0; row < image->height(); ++row)
	{
		for (index_t col = 0; col < image->width(); ++col)
		{
			for (index_t c = 0; c < 4; ++c)
			{
				index_t i = (row * image->width() + col) * 4;

				if (image->getPixel(c, row, col) > 0)
				{
					rawImage[i + c] = (unsigned char)image->getPixel(c, row, col);
				}
				else
				{
					rawImage[i + c] = 0;
				}
			}
		}
	}

	unsigned error = lodepng::encode(png, rawImage, image->width(), image->height());
	if (!error) lodepng::save_file(png, fileName);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;

}