#pragma once
#include "IDX2.h"
#include "common.h"

#include <vector>
#include <list>

namespace Denoise
{
	class SortedPatchCollection
	{
	public:
		SortedPatchCollection();
		SortedPatchCollection(index_t maxNumPatches);
		~SortedPatchCollection();

		void insertPatch(const IDX2& patch);

		std::vector<IDX2>& getPatches();

	private:

		void initialise(index_t maxNumPatches);

		std::vector<IDX2> m_patches;
	};
}