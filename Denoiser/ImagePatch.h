#pragma once

#include "common.h"

struct ImagePatch
{
	ImagePatch(int Col, int Row, int Width, int Height)
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

	int col;
	int row;
	int width;
	int height;

	float * data;
};