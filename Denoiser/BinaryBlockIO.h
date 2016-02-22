#pragma once
#include <string>
#include <vector>

namespace Denoise
{

	class BinaryBlockIO
	{
	public:
		static bool writeBlocks2Disk(const std::string& fileName,
			int blockSize, int numChannels,
			const std::vector<float>& noisyBlocksFreq,
			const std::vector<float>& refBlocksFreq,
			const std::vector<float>& noisyBlocksNoTransform);


		static bool readBlocksFromDisk(const std::string& fileName,
			int& blockSize, int& numChannels,
			std::vector<float>& denoisedBlocksFreq);

	private:
		BinaryBlockIO();
		~BinaryBlockIO();
	};

}



