#pragma once

#include<vector>
#include<string>

struct Dimension
{
	size_t width;
	size_t height;
};

struct Padding
{
	size_t left;
	size_t right;
	size_t top;
	size_t bottom;
};

class Image
{
public:
	Image();
	~Image();

	inline float getPixel(const size_t channel, const size_t idx);
	inline float getPixel(const size_t channel, const size_t row, const size_t col);

	void padImage(Padding& padAmount, bool blackOutside);

	void accessFullImage();
	void accessActualImage();

	inline size_t width();
	inline size_t height();
	inline size_t numChannels();

private:
	Dimension m_fullImageDim;
	Dimension m_actualImageDim;
	Padding m_padding;
	size_t m_numChannels;

	bool m_accessingFullImage;
	std::vector<float*> m_pixelData;

	void printWarning(const std::string& message);
};

float Image::getPixel(const size_t channel, const size_t idx)
{
	return m_pixelData[channel][idx];
}

float Image::getPixel(const size_t channel, const size_t row, const size_t col)
{
	if (m_accessingFullImage)
	{
		return m_pixelData[channel][m_actualImageDim.width * row + col];
	}
	else
	{
		return m_pixelData[channel][m_fullImageDim.width * row + col];
	}
}

size_t Image::width()
{
	if (m_accessingFullImage)
	{
		return m_fullImageDim.width;
	}
	else
	{
		return m_actualImageDim.width;
	}
}

size_t Image::height()
{
	if (m_accessingFullImage)
	{
		return m_fullImageDim.height;
	}
	else
	{
		return m_actualImageDim.height;
	}
}

size_t Image::numChannels()
{
	return m_numChannels;
}

