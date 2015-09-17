#include "SortedPatchCollection.h"

namespace Denoise
{

	SortedPatchCollection::SortedPatchCollection()
	{
		m_patches.resize(32);
	}

	SortedPatchCollection::SortedPatchCollection(size_t maxNumPatches)
	{
		m_patches.resize(maxNumPatches);
	}


	SortedPatchCollection::~SortedPatchCollection()
	{
		m_patches.clear();
	}

	void
	SortedPatchCollection::insertPatch32(const IDX2& patch)
	{
		//Deal with trivial cases
		if (patch.distance > m_patches.back().distance
			|| patch.distance < 0.0f)
		{
			return;
		}

		//if we are only replacing the last element
		if (patch.distance > m_patches[30].distance)
		{
			m_patches[31] = patch;
			return;
		}

		//1.Find out where to insert this patch

		int insertIdx;

		if (patch.distance > m_patches[15].distance) // greater than 16
		{
			if (patch.distance > m_patches[23].distance) // greater than 24
			{
				for (int i = 24; i < 32; ++i)
				{
					if (m_patches[i].distance > patch.distance)
					{
						insertIdx = i;
					}
				}
			}
			else //smaller than 24
			{
				for (int i = 16; i < 24; ++i)
				{
					if (m_patches[i].distance > patch.distance)
					{
						insertIdx = i;
					}
				}
			}
		}
		else //smaller than 16
		{
			if (patch.distance > m_patches[7].distance) // greater than 8
			{
				for (int i = 8; i < 16; ++i)
				{
					if (m_patches[i].distance > patch.distance)
					{
						insertIdx = i;
					}
				}
			}
			else //smaller than 8
			{
				for (int i = 0; i < 8; ++i)
				{
					if (m_patches[i].distance > patch.distance)
					{
						insertIdx = i;
					}
				}
			}
		}

		//2.Insert the patch

		//shift existing patches
		for (int i = insertIdx; i < 32 - 1; ++i)
		{
			m_patches[i + 1] = m_patches[i];
		}

		//insert new patch
		m_patches[insertIdx] = patch;
	}
}