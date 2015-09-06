#include "Image.h"
#include <iostream>

Image::Image()
{
}


Image::~Image()
{
	for (int c = 0; c < m_numChannels; ++c)
	{
		delete[] m_pixelData[c];
	}
}


void Image::padImage(Padding& padAmount, bool blackOutside)
{
	m_fullImageDim.width = m_actualImageDim.width + padAmount.left + padAmount.right;
	m_fullImageDim.height = m_actualImageDim.height + padAmount.top + padAmount.bottom;

	for (int c = 0; c < m_numChannels; ++c)
	{
		//Allocate new buffer;
		float* replacementChannel = new float[m_fullImageDim.width * m_fullImageDim.height];

		//Copy existing data
		for (int row = 0; row < m_actualImageDim.height; ++row)
		{
			for (int col = 0; col < m_actualImageDim.width; ++col)
			{
				int idxOriginal = row * m_actualImageDim.width + col;
				int idxPadded = (row + padAmount.bottom) * m_fullImageDim.width + (col + padAmount.left);
				replacementChannel[idxPadded] = m_pixelData[c][idxOriginal];
			}
		}

		//Mirror pixel values if necessary
		if (!blackOutside)
		{

		}

		//Swap out buffers;
		delete[] m_pixelData[c];
		m_pixelData[c] = replacementChannel;
	}
}

void Image::accessFullImage()
{
	if (m_fullImageDim.width == 0 && m_fullImageDim.height == 0)
	{
		printWarning("This image has not been padded, so accessing the full pixel array makes no difference.");
	}
	else
	{
		m_accessingFullImage = true;
	}
}

void Image::accessActualImage()
{
	m_accessingFullImage = true;
}

void Image::printWarning(const std::string& message)
{
	std::cerr << "Denoise::Image WARNING: [ " << message << " ] " << std::endl;
}
