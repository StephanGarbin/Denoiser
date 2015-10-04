#pragma once

#include <sstream>
#include <string>

namespace Denoise
{
	#define index_t unsigned int
	
	template <typename T>
	std::string Num2String(T value)
	{
		std::stringstream stream;
		stream << value;
		return stream.str();
	}

	template<typename T>
	inline T sqr(T x)
	{
		return x * x;
	}
}