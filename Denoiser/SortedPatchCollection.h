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

		std::vector<IDX2>& getPatches() { return m_patches; }

	private:
		std::vector<IDX2> m_patches;
	};
}