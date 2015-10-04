#pragma once

#include "common.h"

namespace Denoise
{
	struct Dimension
	{
		Dimension()
		{
			width = 0;
			height = 0;
		}

		Dimension(index_t Width, index_t Height)
		{
			width = Width;
			height = Height;
		}

		index_t width;
		index_t height;
	};
}