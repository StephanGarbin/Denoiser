#pragma once
#include "IDX2.h"

#include <vector>
#include <list>

namespace Denoise
{
	class SortedPatchCollection
	{
	public:
		SortedPatchCollection();
		SortedPatchCollection(size_t maxNumPatches);
		~SortedPatchCollection();

		void insertPatch32(const IDX2& patch);

		std::vector<IDX2>& getPatches();

		size_t getNumMeaningfulPatches() { return m_patches.size() - m_numInitialPatchesLeft; }

	private:

		void initialise(size_t maxNumPatches);

		std::vector<IDX2> m_patches;
		size_t m_numInitialPatchesLeft;
	};
}