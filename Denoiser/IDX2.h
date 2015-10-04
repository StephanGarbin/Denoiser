#pragma once

#include "common.h"

namespace Denoise
{
	struct  IDX2
	{
		IDX2()
		{
			row = 0;
			col = 0;
			distance = 0.0f;
		}

		IDX2(index_t Row, index_t Col)
		{
			row = Row;
			col = Col;
			distance = 0.0f;
		}

		IDX2(index_t Row, index_t Col, float Distance)
		{
			row = Row;
			col = Col;
			distance = Distance;
		}

		IDX2(const IDX2& other)
		{
			row = other.row;
			col = other.col;
			distance = other.distance;
		}

		bool operator>(const IDX2& rhs)
		{
			return distance > rhs.distance;
		}

		bool operator<(const IDX2& rhs)
		{
			return distance < rhs.distance;
		}

		friend bool operator>(const IDX2& lhs, const IDX2& rhs)
		{
			return lhs.distance > rhs.distance;
		}

		friend bool operator<(const IDX2& lhs, const IDX2& rhs)
		{
			return lhs.distance < rhs.distance;
		}

		index_t row;
		index_t col;
		float distance;
	};
}