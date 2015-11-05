#pragma once

#include "common.h"
#include "BufferAggregator.h"
#include "Image.h"
#include "IDX2.h"
#include "ImagePatch.h"
#include "BM3DSettings.h"
#include "TBBDefs.h"
#include "BM3DCollaborativeFilterKernel.h"

#include <tbb\blocked_range.h>

namespace Denoise
{

	class BM3DCollaborativeTBB
	{
	public:
		BM3DCollaborativeTBB(Image* image, BufferAggregator& buffer,
			const BM3DSettings& settings, const std::vector<std::vector<IDX2> >& matchedBlocks,
			const ImagePatch& patchTemplate,
			TBB_MUTEX_TYPE& mutex,
			BM3DCollaborativeFilterKernel& kernel);
		~BM3DCollaborativeTBB();

		void operator()(const tbb::blocked_range<size_t>& r) const;

	private:
		Image* m_image;
		BufferAggregator& m_buffer;
		const BM3DSettings& m_settings;
		const ImagePatch& m_patchTemplate;
		const std::vector<std::vector<IDX2> >& m_matchedBlocks;

		BM3DCollaborativeFilterKernel& m_kernel;

		TBB_MUTEX_TYPE& m_mutex;
	};

}

