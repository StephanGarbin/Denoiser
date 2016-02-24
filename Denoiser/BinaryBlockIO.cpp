#include "BinaryBlockIO.h"

#include <fstream>
#include <iostream>
#include <cassert>

namespace Denoise
{

	BinaryBlockIO::BinaryBlockIO()
	{
	}


	BinaryBlockIO::~BinaryBlockIO()
	{
	}

	bool BinaryBlockIO::writeBlocks2Disk(const std::string& fileName,
		int numChannels, int numPatchesPerBlock, int patchSize,
		const std::vector<float>& blocks)
	{
		std::ofstream file(fileName, std::ios::binary);

		if (!file.is_open())
		{
			std::cout << "ERROR: Could not open file [ " << fileName << " ]" << std::endl;
			return false;
		}

		int numFloats = blocks.size();
		int blockSize = numPatchesPerBlock * numChannels;
		int numBlocks = numFloats / (blockSize * patchSize * patchSize);
		
		int numFormat = 0;
		file.write(reinterpret_cast<const char*>(&numFormat), sizeof(numFormat));

		file.write(reinterpret_cast<const char*>(&numBlocks), sizeof(numBlocks));
		file.write(reinterpret_cast<const char*>(&blockSize), sizeof(blockSize));
		file.write(reinterpret_cast<const char*>(&patchSize), sizeof(patchSize));
		file.write(reinterpret_cast<const char*>(&patchSize), sizeof(patchSize));

		file.write(reinterpret_cast<const char*>(&blocks[0]), sizeof(float) * blocks.size());

		file.close();

		std::cout << "Successfully writen Tensor [ " << numBlocks << " x "
		<< numPatchesPerBlock * numChannels << " x " << patchSize << " x "
		<< patchSize << " ] to file [ " << fileName << " ] " << std::endl;

		return true;
	}

	bool BinaryBlockIO::writeBlocks2Disk(const std::string& fileName,
		int numChannels, int numPatchesPerBlock, int patchSize,
		const std::vector<float>& noisyBlocksFreq,
		const std::vector<float>& noisyBlocksNoTransform)	
	{
		//Make sure all buffers are the same size
		assert(noisyBlocksNoTransform.size() == noisyBlocksFreq.size());

		std::ofstream file(fileName, std::ios::binary);

		if (!file.is_open())
		{
			std::cout << "ERROR: Could not open file [ " << fileName << " ]" << std::endl;
			return false;
		}

		int numFloats = noisyBlocksNoTransform.size() * 2;
		int blockSize = numPatchesPerBlock * numChannels * 2;
		int numFloatsPerBlock = blockSize * patchSize * patchSize;
		int numBlocks = numFloats / numFloatsPerBlock;		
		
		std::vector<float> temp(numFloats);


		for(size_t b = 0; b < numBlocks; ++b)
		{
			for(size_t i = 0; i < numFloatsPerBlock / 2; ++i)
			{
				temp[b * numFloatsPerBlock + i] = noisyBlocksFreq[b * (numFloatsPerBlock / 2) + i];
				temp[b * numFloatsPerBlock + (numFloatsPerBlock / 2) + i] = noisyBlocksNoTransform[b * (numFloatsPerBlock / 2) + i];
			}
		}

		int numFormat = 0;
		file.write(reinterpret_cast<const char*>(&numFormat), sizeof(numFormat));
		file.write(reinterpret_cast<const char*>(&numBlocks), sizeof(numBlocks));
		file.write(reinterpret_cast<const char*>(&blockSize), sizeof(blockSize));
		file.write(reinterpret_cast<const char*>(&patchSize), sizeof(patchSize));
		file.write(reinterpret_cast<const char*>(&patchSize), sizeof(patchSize));

		file.write(reinterpret_cast<const char*>(&temp[0]), sizeof(float) * temp.size());

		file.close();
		
		temp.clear();

		std::cout << "Successfully writen Tensor [ " << numBlocks << " x "
		<< numPatchesPerBlock * numChannels << " x " << patchSize << " x "
		<< patchSize << " ] to file [ " << fileName << " ] " << std::endl;

		return true;
	}	

	bool BinaryBlockIO::readBlocksFromDisk(const std::string& fileName,
		int& numChannels, int& numPatchesPerBlock, int& patchSize,
		std::vector<float>& denoisedBlocksFreq)
	{
		std::ifstream file(fileName, std::ios::binary);

		if (!file.is_open())
		{
			std::cout << "ERROR: Could not open file [ " << fileName << " ]" << std::endl;
			return false;
		}

		int numBlocks;
		int numFormat;
		int blockSize;
		file.read(reinterpret_cast<char*>(&numFormat), sizeof(numFormat));
		assert(numFormat == 0);
		
		file.read(reinterpret_cast<char*>(&numBlocks), sizeof(numBlocks));
		file.read(reinterpret_cast<char*>(&blockSize), sizeof(blockSize));
		file.read(reinterpret_cast<char*>(&patchSize), sizeof(patchSize));
		file.read(reinterpret_cast<char*>(&patchSize), sizeof(patchSize));

		int numFloats = numBlocks * blockSize * patchSize * patchSize;

		denoisedBlocksFreq.resize(numFloats);

		file.read(reinterpret_cast<char*>(&denoisedBlocksFreq[0]), sizeof(float)* denoisedBlocksFreq.size());

		file.close();

		std::cout << "Successfully read Tensor [ " << numBlocks << " x "
		<< numPatchesPerBlock * numChannels << " x " << patchSize << " x "
		<< patchSize << " ] from file [ " << fileName << " ] " << std::endl;

		return true;
	}

}
