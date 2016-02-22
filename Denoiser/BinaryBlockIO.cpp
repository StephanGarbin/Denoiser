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
		int blockSize, int numChannels,
		const std::vector<float>& noisyBlocksFreq,
		const std::vector<float>& refBlocksFreq,
		const std::vector<float>& noisyBlocksNoTransform)
	{
		//Make sure all buffers are the same size
		assert(noisyBlocksNoTransform.size() == noisyBlocksFreq.size() && noisyBlocksNoTransform.size() == refBlocksFreq.size());

		std::ofstream file(fileName, std::ios::binary);

		if (!file.is_open())
		{
			std::cout << "ERROR: Could not open file [ " << fileName << " ]" << std::endl;
			return false;
		}

		int numBlocks = noisyBlocksNoTransform.size();

		file.write(reinterpret_cast<const char*>(&numBlocks), sizeof(numBlocks));
		file.write(reinterpret_cast<const char*>(&blockSize), sizeof(blockSize));
		file.write(reinterpret_cast<const char*>(&numChannels), sizeof(numChannels));

		file.write(reinterpret_cast<const char*>(&noisyBlocksFreq[0]), sizeof(float)* noisyBlocksFreq.size());
		file.write(reinterpret_cast<const char*>(&refBlocksFreq[0]), sizeof(float)* refBlocksFreq.size());
		file.write(reinterpret_cast<const char*>(&noisyBlocksNoTransform[0]), sizeof(float)* noisyBlocksNoTransform.size());

		file.close();

		return true;
	}


	bool BinaryBlockIO::readBlocksFromDisk(const std::string& fileName,
		int& blockSize, int& numChannels,
		std::vector<float>& denoisedBlocksFreq)
	{
		std::ifstream file(fileName, std::ios::binary);

		if (!file.is_open())
		{
			std::cout << "ERROR: Could not open file [ " << fileName << " ]" << std::endl;
			return false;
		}

		int numBlocks;

		file.read(reinterpret_cast<char*>(&numBlocks), sizeof(numBlocks));
		file.read(reinterpret_cast<char*>(&blockSize), sizeof(blockSize));
		file.read(reinterpret_cast<char*>(&numChannels), sizeof(numChannels));

		int totalBlockSize = blockSize * numChannels;

		std::cout << "Num Blocks to read: " << numBlocks
			<< "; Num Channels: " << numChannels << "; Block Size: " << blockSize << std::endl;

		denoisedBlocksFreq.resize(numBlocks);

		file.read(reinterpret_cast<char*>(&denoisedBlocksFreq[0]), sizeof(float)* denoisedBlocksFreq.size());

		file.close();

		return true;
	}

}

