#include "SortedPatchCollection.h"

#include <limits>
#include <algorithm>
#include <iostream>

namespace Denoise
{

	SortedPatchCollection::SortedPatchCollection()
	{
		initialise(32);
	}

	SortedPatchCollection::SortedPatchCollection(index_t maxNumPatches)
	{
		initialise(maxNumPatches);
	}

	void SortedPatchCollection::initialise(index_t maxNumPatches)
	{
		m_patches.resize(maxNumPatches);
		for (index_t i = 0; i < m_patches.size(); ++i)
		{
			m_patches[i].distance = std::numeric_limits<float>::max();
		}
	}

	SortedPatchCollection::~SortedPatchCollection()
	{
		m_patches.clear();
	}

	void SortedPatchCollection::insertPatch(const IDX2& patch)
	{
		if (patch.distance >= (--m_patches.end())->distance)
		{
			return;
		}

		//1. Find Insertion Place
		std::vector<IDX2>::iterator insertLocation = std::upper_bound<std::vector<IDX2>::iterator>(m_patches.begin(), m_patches.end(), patch);

		//2. Shift Vector
		for (int i = m_patches.size() - 1; i > std::distance<std::vector<IDX2>::iterator>(m_patches.begin(), insertLocation); --i)
		{
			m_patches[i] = m_patches[i - 1];
		}

		//3. Update Vector
		*insertLocation = patch;
	}


	std::vector<IDX2>& SortedPatchCollection::getPatches()
	{
		return m_patches;
	}
}