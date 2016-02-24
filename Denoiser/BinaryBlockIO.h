#pragma once
#include <string>
#include <vector>

namespace Denoise
{

	class BinaryBlockIO
	{
	public:
		static bool writeBlocks2Disk(const std::string& fileName,
			int numChannels, int numPatchesPerBlock, int patchSize,
			const std::vector<float>& blocks);

		static bool writeBlocks2Disk(const std::string& fileName,
			int numChannels, int numPatchesPerBlock, int patchSize,
			const std::vector<float>& noisyBlocksFreq,
			const std::vector<float>& noisyBlocksNoTransform);

		static bool readBlocksFromDisk(const std::string& fileName,
			int& numChannels, int& numPatchesPerBlock, int& patchSize,
			std::vector<float>& denoisedBlocksFreq);

	private:
		BinaryBlockIO();
		~BinaryBlockIO();
	};

}


