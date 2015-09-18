#include "SortedPatchCollection.h"

#include <limits>
#include <algorithm>
#include <iostream>

namespace Denoise
{

	SortedPatchCollection::SortedPatchCollection()
	{
		m_patches.resize(32);
		for (size_t i = 0; i < m_patches.size(); ++i)
		{
			m_patches[i].distance = std::numeric_limits<float>::max();
		}
		//for (int i = 0; i < 32; ++i)
		//{
		//	m_patchesList.push_back(IDX2(0, 0, std::numeric_limits<float>::max()));
		//}
	}

	SortedPatchCollection::SortedPatchCollection(size_t maxNumPatches)
	{
		
	}


	SortedPatchCollection::~SortedPatchCollection()
	{

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
			

			//for (std::vector<IDX2>::iterator it = --m_patches.end(); it != insertLocation; --it)
			//{
			//	std::swap(--it, it);
			//}


			//3. Update Vector
			*insertLocation = patch;

		////1. Find Insertion Place
		//std::list<IDX2>::iterator insertLocation = std::upper_bound<std::list<IDX2>::iterator>(m_patchesList.begin(), m_patchesList.end(), patch);

		//if (insertLocation == m_patchesList.end())
		//{
		//	return;
		//}
		////2. Shift List
		//m_patchesList.back() = patch;
		//m_patchesList.splice(insertLocation, m_patchesList, --m_patchesList.end());
	}


	std::vector<IDX2>& SortedPatchCollection::getPatches()
	{
		//for (std::list<IDX2>::iterator it = m_patchesList.begin(); it != m_patchesList.end(); ++it)
		//{
		//	m_patches.push_back(*it);
		//}

		return m_patches;
	}

}