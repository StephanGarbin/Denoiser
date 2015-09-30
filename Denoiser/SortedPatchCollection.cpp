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

	SortedPatchCollection::SortedPatchCollection(size_t maxNumPatches)
	{
		initialise(maxNumPatches);
	}

	void SortedPatchCollection::initialise(size_t maxNumPatches)
	{
		m_patches.resize(maxNumPatches);
		for (size_t i = 0; i < m_patches.size(); ++i)
		{
			m_patches[i].distance = std::numeric_limits<float>::max();
		}

		m_numInitialPatchesLeft = m_patches.size();
	}

	SortedPatchCollection::~SortedPatchCollection()
	{
		m_patches.clear();
	}

	void SortedPatchCollection::insertPatch32(const IDX2& patch)
	{
		//1. Find Insertion Place
		std::vector<IDX2>::iterator insertLocation = std::upper_bound<std::vector<IDX2>::iterator>(m_patches.begin(), m_patches.end(), patch);

		if (insertLocation == m_patches.end())
		{
			return;
		}

		//2. Shift Vector
		for (int i = 31; i > std::distance<std::vector<IDX2>::iterator>(m_patches.begin(), insertLocation); --i)
		{
			m_patches[i] = m_patches[i - 1];
		}

		//3. Update Vector
		*insertLocation = patch;

		if (m_numInitialPatchesLeft > 0)
		{
			--m_numInitialPatchesLeft;
		}
	}


	std::vector<IDX2>& SortedPatchCollection::getPatches()
	{
		return m_patches;
	}
}