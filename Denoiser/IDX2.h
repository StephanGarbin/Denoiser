#pragma once

struct  IDX2
{
	IDX2()
	{
		row = 0;
		col = 0;
		distance = 0.0f;
	}

	IDX2(size_t Row, size_t Col)
	{
		row = Row;
		col = Col;
		distance = 0.0f;
	}

	IDX2(size_t Row, size_t Col, float Distance)
	{
		row = Row;
		col = Col;
		distance = Distance;
	}

	bool operator>(const IDX2& rhs)
	{
		return distance > rhs.distance;
	}

	bool operator<(const IDX2& rhs)
	{
		return distance < rhs.distance;
	}

	size_t row;
	size_t col;
	float distance;
};