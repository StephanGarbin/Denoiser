#pragma once

#include "common.h"

struct ImagePatch
{
	ImagePatch(index_t Col, index_t Row, index_t Width, index_t Height)
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

	index_t col;
	index_t row;
	index_t width;
	index_t height;

	float * data;
};