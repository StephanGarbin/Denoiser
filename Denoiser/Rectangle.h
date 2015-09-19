#pragma once

namespace Denoise
{
	struct Rectangle
	{
		Rectangle(int Left, int Right, int Top, int Bottom)
		{
			left = Left;
			right = Right;
			top = Top;
			bottom = Bottom;
		}

		Rectangle()
		{
			left = 0;
			right = 0;
			top = 0;
			bottom = 0;
		}

		int left;
		int right;
		int top;
		int bottom;

		int width() const
		{
			return right - left;
		}

		int height() const
		{
			return top - bottom;
		}

		int size() const
		{
			return width() * height();
		}
	};
}