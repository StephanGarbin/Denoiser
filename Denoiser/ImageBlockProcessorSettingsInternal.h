#pragma once

#include "common.h"
#include "Rectangle.h"

namespace Denoise
{
	struct ImageBlockProcessorSettingsInternal
	{
		int shiftRows;
		int shiftCols;

		index_t offsetRows;
		index_t offsetCols;

		Rectangle accessibleImageBlock;
		Rectangle accessibleImageBlockShifted;

		bool iterateAtBorders;
	};
}