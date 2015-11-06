#pragma once

#include <memory>

#include "common.h"
#include "BufferAggregator.h"
#include "Image.h"
#include "IDX2.h"
#include "ImagePatch.h"
#include "BM3DSettings.h"
#include "TBBDefs.h"
#include "BM3DWienerFilterKernel.h"

#include <tbb\blocked_range.h>

namespace Denoise
{

	class BM3DWienerTBB
	{
	public:
		BM3DWienerTBB(Image* image, Image* basicImage, BufferAggregator& buffer,
			const BM3DSettings& settings, const std::vector<std::vector<IDX2> >& matchedBlocks,
			const ImagePatch& patchTemplate,
			TBB_MUTEX_TYPE& mutex);

		BM3DWienerTBB(const BM3DWienerTBB& other);

		~BM3DWienerTBB();

		void operator()(const tbb::blocked_range<size_t>& r) const;

	private:
		Image* m_image;
		Image* m_basicImage;
		BufferAggregator& m_buffer;
		const BM3DSettings& m_settings;
		const ImagePatch& m_patchTemplate;
		const std::vector<std::vector<IDX2> >& m_matchedBlocks;

		std::shared_ptr<BM3DWienerFilterKernel> m_kernel;

		TBB_MUTEX_TYPE& m_mutex;
	};

}

