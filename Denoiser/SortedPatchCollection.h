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

		void insertPatch(const IDX2& patch);

		std::vector<IDX2>& getPatches();

	private:

		void initialise(size_t maxNumPatches);

		std::vector<IDX2> m_patches;
	};
}