#include <vector>
#include <string>
#include <iostream>

#include "lodepng.h"

#include "Image.h"
#include "ImageBlockProcessor.h"
#include "IDX2.h"
#include "Rectangle.h"
#include "ImagePatch.h"

#include "BM3DImageBlockProcessorNN.h"
#include "BM3DSettings.h"
#include "BinaryBlockIO.h"

int main(int argc, char* argv[])
{
	float stdDeviation;
	index_t numThreadsBM;
	float adaptiveFactor = 0.0f;
	std::string inputReferenceFile;
	std::string inputFile;
	std::string outputFile;
	std::string binaryFile;
	bool read;
	float perc;

	if (argc < 8)
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
		inputReferenceFile = std::string(argv[5]);
		binaryFile = std::string(argv[6]);

		read = std::string(argv[7]) == "READ";

		if (argc > 8)
		{
			perc = std::atof(argv[8]);
		}
		else
		{
			perc = 100.0f;
		}
	}


	Denoise::Image* image = new Denoise::Image();
	image->readFromFile(inputFile);

	Denoise::Image* imageReference = new Denoise::Image();
	imageReference->readFromFile(inputReferenceFile);

	image->checkImageIntegrity(true);
	image->normalise();

	imageReference->checkImageIntegrity(true);
	imageReference->normalise();

	Denoise::Image result(*image);
	Denoise::Image basic(*image);

	std::cout << "Filtering..." << std::endl;

	Denoise::BM3DImageBlockProcessorNN bm3dFilter(image, &basic, &result, imageReference);

	Denoise::BM3DSettings bm3dFilterSettings;

	std::vector<float> smoothness;
	smoothness.push_back(stdDeviation);
	smoothness.push_back(stdDeviation);
	smoothness.push_back(stdDeviation);

	bm3dFilterSettings.init2defaults(smoothness, false);
	bm3dFilterSettings.limitHardwareConcurrency(numThreadsBM);

	std::vector<float> noisyBlocksNoTransform;
	std::vector<float> noisyBlocksFreq;
	std::vector<float> refBlocksFreq;

	int blockSize = bm3dFilterSettings.patchSize * bm3dFilterSettings.patchSize * bm3dFilterSettings.numPatchesPerBlockCollaborative;
	int numChannels = image->numChannels();

	if (read)
	{
		std::cout << "Loading Blocks from file [ " << binaryFile << " ]" << std::endl;
		Denoise::BinaryBlockIO::readBlocksFromDisk(binaryFile, blockSize, numChannels, noisyBlocksFreq);
	}

	bm3dFilter.process(bm3dFilterSettings, read, noisyBlocksFreq, refBlocksFreq, noisyBlocksNoTransform, perc, true);

	if (!read)
	{
		std::cout << "Writing Blocks to file [ " << binaryFile << " ]";
		Denoise::BinaryBlockIO::writeBlocks2Disk(binaryFile, blockSize, numChannels, noisyBlocksFreq, refBlocksFreq,
			noisyBlocksNoTransform);
	}

	image->undoNormalise();
	result.undoNormalise();
	basic.undoNormalise();

	basic.setAlphaToOne();
	result.setAlphaToOne();

	basic.save2File(outputFile);

	delete image;
	delete imageReference;
}
