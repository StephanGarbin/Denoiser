#include <sstream>
#include <string>

namespace Denoise
{
	template <typename T>
	std::string Num2String(T value)
	{
		std::stringstream stream;
		stream << value;
		return stream.str();
	}

	inline int sqr(int x)
	{
		return x * x;
	}
}