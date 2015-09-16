#pragma once

namespace Denoise
{
	class Rectangle
	{
	public:
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