#include "SortedPatchCollection.h"

#include <limits>
#include <algorithm>

namespace Denoise
{

	SortedPatchCollection::SortedPatchCollection()
	{
		m_patches.resize(32);
		for (size_t i = 0; i < m_patches.size(); ++i)
		{
			m_patches[i].distance = std::numeric_limits<float>::max();
		}
	}

	SortedPatchCollection::SortedPatchCollection(size_t maxNumPatches)
	{
		
	}


	SortedPatchCollection::~SortedPatchCollection()
	{

	}

	void
		SortedPatchCollection::insertPatch32(const IDX2& patch)
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
	}

}