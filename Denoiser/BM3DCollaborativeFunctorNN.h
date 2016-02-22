#pragma once

#include <memory>

#include "common.h"
#include "BufferAggregator.h"
#include "Image.h"
#include "IDX2.h"
#include "ImagePatch.h"
#include "BM3DSettings.h"
#include "Definitions.h"
#include "BM3DCollaborativeFilterKernel.h"

namespace Denoise
{

	class BM3DCollaborativeFunctorNN
	{
	public:
		BM3DCollaborativeFunctorNN(Image* image, BufferAggregator& buffer,
			const BM3DSettings& settings, const std::vector<std::vector<IDX2> >& matchedBlocks,
			const ImagePatch& patchTemplate,
			MUTEX_TYPE& mutex);

		BM3DCollaborativeFunctorNN(const BM3DCollaborativeFunctorNN& other);

		~BM3DCollaborativeFunctorNN();

		void operator()(const std::pair<size_t, size_t>& r, float* destination, int* destinationIdxs,
			bool loadBlocks,
			float* untransformedRef = nullptr) const;

	private:
		Image* m_image;
		BufferAggregator& m_buffer;
		const BM3DSettings& m_settings;
		const ImagePatch& m_patchTemplate;
		const std::vector<std::vector<IDX2> >& m_matchedBlocks;

		std::shared_ptr<BM3DCollaborativeFilterKernel> m_kernel;

		MUTEX_TYPE& m_mutex;
	};

}

