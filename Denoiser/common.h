#include <sstream>
#include <string>

template <typename T>
std::string Num2String(T value)
{
	std::stringstream stream;
	stream << value;
	return stream.str();
}