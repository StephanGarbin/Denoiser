#pragma once

namespace Denoise
{
	struct Dimension
	{
		Dimension()
		{
			width = 0;
			height = 0;
		}

		Dimension(size_t Width, size_t Height)
		{
			width = Width;
			height = Height;
		}

		size_t width;
		size_t height;
	};
}