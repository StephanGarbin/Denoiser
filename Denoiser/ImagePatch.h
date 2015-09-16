#pragma once


struct ImagePatch
{
	size_t col;
	size_t row;
	size_t width;
	size_t height;

	float * data;
};