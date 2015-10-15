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
	if (argc != 2)
	{
		std::cout << "Error: Please provide standard deviation!" << std::endl;
		return 0;
	}

	//std::string inputFile = "C:/Users/Stephan/Desktop/tiger.png";
	//std::string inputFile = "C:/Users/Stephan/Desktop/llama.png";
	//std::string outputFile = "C:/Users/Stephan/Desktop/llama_padded.png";
	//std::string inputFile = "C:/Users/Stephan/Desktop/noisyTrees2.png";
	//std::string outputFile = "C:/Users/Stephan/Desktop/noisyTreesNew2b.png";

	std::string inputFile = "C:/Users/Stephan/Desktop/noisyTrees.png";
	std::string outputFile = "C:/Users/Stephan/Desktop/noisyTreesNew.png";

	//std::string inputFile = "C:/Users/Stephan/Desktop/computerNoisy.png";
	//std::string outputFile = "C:/Users/Stephan/Desktop/computerNew.png";

	//std::string inputFile = "C:/Users/Stephan/Desktop/RendermanTestScene1.png";
	//std::string outputFile = "C:/Users/Stephan/Desktop/RendermanTestScene1BM3D.png";

	float stdDeviation = std::atof(argv[1]);

	Denoise::Image* image = nullptr;

	loadImage(&image, inputFile);
	image->checkImageIntegrity(true);
	image->normalise();
	/*Padding pad;
	pad.bottom = 10;
	pad.top = 20;
	pad.left = 20;
	pad.right = 10;
	image->padImage(pad, false);
	//image->accessFullImage();*/
	/*int idx = 150;
	std::vector<std::vector<Denoise::IDX2> > similarPatches;
	std::vector<Denoise::IDX2> similarPatchesComparison;
	ImagePatch templatePatch;
	templatePatch.height = 6;
	templatePatch.width = 6;
	templatePatch.col = 0;
	templatePatch.row = 0;
	Denoise::IDX2 singlePosition(idx, idx);
	Denoise::ImageBlockProcessor blockProcess(*image);
	Denoise::Rectangle block;
	block.left = 0;
	block.bottom = 0;
	block.right = image->width();
	block.top = image->height();
	std::cout << "Processing Integral Method..." << std::endl;
	tbb::tick_count start = tbb::tick_count::now();
	blockProcess.computeNMostSimilar(templatePatch, block, 1, 1, 30, 30, 32, 0.1f, 2, similarPatches);
	tbb::tick_count end = tbb::tick_count::now();
	std::cout << "Time: " << (end - start).seconds() << "s." << std::endl;
	std::vector<std::vector<Denoise::IDX2> > similarPatchesComparisonArray;
	similarPatchesComparisonArray.resize(block.size() * 2);
	for (int i = 0; i < similarPatchesComparisonArray.size(); ++i)
	{
		similarPatchesComparisonArray[i].reserve(45);
	}
	std::cout << "Processing Naive Method..." << std::endl;
	tbb::tick_count start2 = tbb::tick_count::now();
	for (int row = 20; row < block.height() - 20; ++row)
	{
		for (int col = 20; col < block.width() - 20; ++col)
		{
			Denoise::IDX2 position(row, col);
			blockProcess.computeNMostSimilarNaive(similarPatchesComparisonArray[row * (block.width()) + col], position, templatePatch, 30, 30, 32, 0.1f, 2);
		}
	}
	tbb::tick_count end2 = tbb::tick_count::now();
	std::cout << "Time: " << (end2 - start2).seconds() << "s." << std::endl;
	blockProcess.computeNMostSimilarNaive(similarPatchesComparison, singlePosition, templatePatch, 30, 30, 32, 10000.0f, 2);
	std::cout << "Reference: " << std::endl;
	for (int i = 0; i < similarPatchesComparison.size(); ++i)
	{
		std::cout << "(" << similarPatchesComparison[i].col << ", " << similarPatchesComparison[i].row
			<< ", " << similarPatchesComparison[i].distance << ");  ";
	}
	std::cout << std::endl << "New: " << std::endl;
	for (int i = 0; i < similarPatches[image->width() * idx + idx].size(); ++i)
	{
		std::cout << "(" << similarPatches[image->width() * idx + idx][i].col << ", "
			<< similarPatches[image->width() * idx + idx][i].row << ", "
			<< similarPatches[image->width() * idx + idx][i].distance << ");  ";
	}*/

	//Denoise::Image result(image->actualDimension(), image->format());
	Denoise::Image result(*image);

	//Denoise::ImageNLMeansProcessor nlMeansFilter(image, &result);

	//Denoise::NLMeansSettings nlMeansFilterSettings;
	//nlMeansFilterSettings.maxAllowedPatchDistance = 10.8f;
	//nlMeansFilterSettings.numPatchesPerBlock = 32;
	//nlMeansFilterSettings.patchSize = 3;
	//nlMeansFilterSettings.searchWindowSize = 20;
	//nlMeansFilterSettings.stepSizeCols = 1;
	//nlMeansFilterSettings.stepSizeRows = 1;
	//nlMeansFilterSettings.usePatchWeighting = true;
	//nlMeansFilterSettings.stdDeviation = stdDeviation;
	//nlMeansFilterSettings.filteringParameter = 0.55f;
	//nlMeansFilter.process(nlMeansFilterSettings, true);

	Denoise::BM3DImageBlockProcessor bm3dFilter(image, &result);

	Denoise::BM3DSettings bm3dFilterSettings;
	bm3dFilterSettings.maxAllowedPatchDistance = 1.81f;
	bm3dFilterSettings.numPatchesPerBlock = 16;
	bm3dFilterSettings.patchSize = 8;
	bm3dFilterSettings.searchWindowSize = 32;
	bm3dFilterSettings.stepSizeCols = 3;
	bm3dFilterSettings.stepSizeRows = 3;
	bm3dFilterSettings.usePatchWeighting = false;
	bm3dFilterSettings.stdDeviation = stdDeviation;
	bm3dFilterSettings.averageBlocksBasedOnStd = true;
	bm3dFilterSettings.averageBlocksBasedOnStdFactor = 0.001f;

	bm3dFilter.process(bm3dFilterSettings, true);

	image->undoNormalise();
	result.undoNormalise();

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