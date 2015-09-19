#pragma once


struct ImagePatch
{
	ImagePatch(size_t Col, size_t Row, size_t Width, size_t Height)
	{
		col = Col;
		row = Row;
		width = Width;
		height = Height;
		data = nullptr;
	}

	ImagePatch()
	{
		col = 0;
		row = 0;
		width = 0;
		height = 0;
		data = nullptr;
	}

	size_t col;
	size_t row;
	size_t width;
	size_t height;

	float * data;
};