#include "Image.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <functional>

#include "common.h"
namespace Denoise
{

	Image::Image(const Dimension& imageDimension, size_t format)
	{
		initialise(imageDimension, format);
	}


	Image::Image(const Image& other)
	{
		m_fullImageDim = other.m_fullImageDim;
		m_actualImageDim = other.m_actualImageDim;
		m_padding = other.m_padding;
		m_numChannels = other.m_numChannels;

		m_accessingFullImage = other.m_accessingFullImage;
		m_isPadded = other.m_isPadded;

		m_format = other.m_format;

		m_verbosityLevel = other.m_verbosityLevel;

		m_isNormalised = other.m_isNormalised;
		m_normalisationValue = other.m_normalisationValue;

		m_pixelData.resize(other.m_pixelData.size());
		for (size_t c = 0; c < m_pixelData.size(); ++c)
		{
			m_pixelData[c] = new float[m_fullImageDim.width * m_fullImageDim.height];
			for (size_t i = 0; i < m_fullImageDim.width * m_fullImageDim.height; ++i)
			{
				m_pixelData[c][i] = other.m_pixelData[c][i];
			}
		}
	}


	Image::~Image()
	{
		for (size_t c = 0; c < m_numChannels; ++c)
		{
			delete[] m_pixelData[c];
		}
	}

	void Image::initialise(const Dimension& imageDimension, size_t format)
	{
			m_actualImageDim = imageDimension;
			m_fullImageDim = imageDimension;
			m_numChannels = format;
			m_format = format;

			m_isNormalised = false;
			m_verbosityLevel = 3;
			m_accessingFullImage = false;
			m_isPadded = false;

			//resize internal arrays
			m_pixelData.resize(format + 1);
			for (size_t c = 0; c < m_pixelData.size(); ++c)
			{
				m_pixelData[c] = new float[imageDimension.width * imageDimension.height];
			}

			m_normalisationValue = 1.0f;
		}

	bool Image::padImage(Padding& padAmount, bool blackOutside)
	{
		if (padAmount.bottom > m_fullImageDim.height / 2
			|| padAmount.top > m_fullImageDim.height / 2
			|| padAmount.left > m_fullImageDim.width / 2
			|| padAmount.right > m_fullImageDim.width / 2)
		{
			printError("Cannot pad image as image dimension has to be twice as large as pad amount!");
			return false;
		}

		m_padding = padAmount;

		m_fullImageDim.width = m_actualImageDim.width + padAmount.left + padAmount.right;
		m_fullImageDim.height = m_actualImageDim.height + padAmount.top + padAmount.bottom;

		for (size_t c = 0; c < m_numChannels; ++c)
		{
			//Allocate new buffer;
			float* replacementChannel = new float[m_fullImageDim.width * m_fullImageDim.height];

			//Copy existing data
			for (size_t row = 0; row < m_actualImageDim.height; ++row)
			{
				for (size_t col = 0; col < m_actualImageDim.width; ++col)
				{
					size_t idxOriginal = row * m_actualImageDim.width + col;
					size_t idxPadded = (row + padAmount.top) * m_fullImageDim.width + (col + padAmount.left);
					replacementChannel[idxPadded] = m_pixelData[c][idxOriginal];
				}
			}

			//Swap pointers
			delete[] m_pixelData[c];
			m_pixelData[c] = replacementChannel;

			//Mirror pixel values if necessary
			size_t counter = padAmount.left;
			//left
			if (padAmount.left > 0)
			{
				for (size_t col = 0; col < padAmount.left; ++col)
				{
					for (size_t row = 0; row < m_fullImageDim.height; ++row)
					{
						if (!blackOutside)
						{
							m_pixelData[c][IDX2_2_1(row, col)] = m_pixelData[c][IDX2_2_1(row, padAmount.left + counter)];
						}
						else
						{
							m_pixelData[c][IDX2_2_1(row, col)] = 0.0f;
						}
					}
					--counter;
				}
			}

			//top
			if (padAmount.top > 0)
			{
				counter = 1;
				for (size_t row = padAmount.top - 1; row > 0; --row)
				{
					for (size_t col = 0; col < m_fullImageDim.width; ++col)
					{
						if (!blackOutside)
						{
							m_pixelData[c][IDX2_2_1(row, col)] = m_pixelData[c][IDX2_2_1((padAmount.top - 1) + counter, col)];
						}
						else
						{
							m_pixelData[c][IDX2_2_1(row, col)] = 0.0f;
						}
					}
					++counter;
				}
			}

			//right
			if (padAmount.right > 0)
			{
				counter = 1;
				for (size_t col = m_fullImageDim.width - padAmount.right; col < m_fullImageDim.width; ++col)
				{
					for (size_t row = 0; row < m_fullImageDim.height; ++row)
					{
						if (!blackOutside)
						{
							m_pixelData[c][IDX2_2_1(row, col)] = m_pixelData[c][IDX2_2_1(row, m_fullImageDim.width - (padAmount.right + counter))];
						}
						else
						{
							m_pixelData[c][IDX2_2_1(row, col)] = 0.0f;
						}
					}
					++counter;
				}
			}

			//bottom
			if (padAmount.bottom > 0)
			{
				counter = 1;
				for (size_t row = m_fullImageDim.height - padAmount.bottom; row < m_fullImageDim.height; ++row)
				{
					for (size_t col = 0; col < m_fullImageDim.width; ++col)
					{
						if (!blackOutside)
						{
							m_pixelData[c][IDX2_2_1(row, col)] = m_pixelData[c][IDX2_2_1((m_fullImageDim.height - padAmount.bottom - 1) - counter, col)];
						}
						else
						{
							m_pixelData[c][IDX2_2_1(row, col)] = 0.0f;
						}
					}
					++counter;
				}
			}
		}

		m_isPadded = true;
		return true;
	}

	void Image::accessFullImage()
	{
		if (m_fullImageDim.width == m_actualImageDim.width && m_fullImageDim.height == m_actualImageDim.height)
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
		m_accessingFullImage = false;
	}

	void Image::printWarning(const std::string& message)
	{
		if (m_verbosityLevel > 1)
		{
			std::cerr << "Denoise::Image WARNING: [ " << message << " ] " << std::endl;
		}
	}

	void Image::printError(const std::string& message)
	{
		if (m_verbosityLevel > 0)
		{
			std::cerr << "Denoise::Image ERROR: [ " << message << " ] " << std::endl;
		}
	}

	void Image::printNotification(const std::string& message)
	{
		if (m_verbosityLevel > 2)
		{
			std::cerr << "Denoise::Image NOTIFICATION: [ " << message << " ] " << std::endl;
		}
	}

	void Image::normalise()
	{
		std::vector<float> maxPixelsValuesPerChannel(m_pixelData.size());
		for (size_t c = 0; c < maxPixelsValuesPerChannel.size(); ++c)
		{
			maxPixelsValuesPerChannel[c] = maxPixelValue(c);
		}

		std::sort(maxPixelsValuesPerChannel.begin(), maxPixelsValuesPerChannel.end(), std::greater<float>());

		m_normalisationValue = maxPixelsValuesPerChannel[0];

		if (m_normalisationValue == 0.0f)
		{
			printError("Cannot normalise. Maximum Pixel Value is 0.0.");
		}
		else
		{
			m_isNormalised = true;
			for (size_t c = 0; c < m_pixelData.size(); ++c)
			{
				for (size_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
				{
					m_pixelData[c][i] /= m_normalisationValue;
				}
			}
		}
	}

	void Image::undoNormalise(float normalisationValue)
	{
		if (!m_isNormalised)
		{
			printWarning("Image is not normalised, so un-doing normalisation has no effect.");
			return;
		}

		for (size_t c = 0; c < m_pixelData.size(); ++c)
		{
			for (size_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
			{
				if (normalisationValue == -1.0f)
				{
					m_pixelData[c][i] *= m_normalisationValue;
				}
				else
				{
					m_pixelData[c][i] *= normalisationValue;
				}
			}
		}

		m_isNormalised = false;
		m_normalisationValue = 1.0f;
	}

	float Image::maxPixelValue(size_t channel) const
	{
		float maxValue = std::numeric_limits<float>::min();
		for (size_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
		{
			maxValue = (m_pixelData[channel][i] > maxValue) ? m_pixelData[channel][i] : maxValue;
		}
		return maxValue;
	}

	float Image::minPixelValue(size_t channel) const
	{
		float minValue = std::numeric_limits<float>::max();
		for (size_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
		{
			minValue = (m_pixelData[channel][i] < minValue) ? m_pixelData[channel][i] : minValue;
		}
		return minValue;
	}

	float Image::averagePixelValue(size_t channel)
	{
		//do accumulation in double to avoid precision errors
		long double pixelSum = 0.0;
		for (size_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
		{
			pixelSum += (double)m_pixelData[channel][i];
		}
		pixelSum /= (double)(m_fullImageDim.height * m_fullImageDim.width);

		return (float)pixelSum;
	}

	bool Image::checkImageIntegrity(bool enforceIntegrity)
	{
		size_t containsNegative = 0;
		size_t containsNaNs = 0;
		size_t containsInfs = 0;

		for (size_t c = 0; c < m_pixelData.size(); ++c)
		{
			for (size_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
			{
				if (std::isnan(m_pixelData[c][i]))
				{
					++containsNaNs;
				}
				else if (std::isinf(m_pixelData[c][i]))
				{
					++containsInfs;
				}
				else if (m_pixelData[c][i] < 0.0f)
				{
					++containsNegative;
				}
			}
		}

		if (containsNaNs > 0)
		{
			printWarning("Image contains " + Num2String(containsNaNs) + " NaN values.");
		}
		if (containsInfs > 0)
		{
			printWarning("Image contains " + Num2String(containsInfs) + " Inf values.");
		}
		if (containsNegative > 0)
		{
			printWarning("Image contains " + Num2String(containsNegative) + " negative values.");
		}

		if (enforceIntegrity)
		{
			for (size_t c = 0; c < m_pixelData.size(); ++c)
			{
				for (size_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
				{
					if (std::isnan(m_pixelData[c][i]))
					{
						m_pixelData[c][i] = 0.0f;
					}
					else if (std::isinf(m_pixelData[c][i]))
					{
						m_pixelData[c][i] = 0.0f;
					}
					else if (m_pixelData[c][i] < 0.0f)
					{
						m_pixelData[c][i] = 0.0f;
					}
				}
			}

			printNotification("Image integrity enforced (All incompatible pixel values ("
				+ Num2String(containsNegative + containsInfs + containsNaNs) + ") set to 0.");
		}

		if (containsInfs > 0 || containsNaNs > 0 || containsNegative > 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	void Image::cpy2Block3d(const std::vector<IDX2>& patches, float* block, const ImagePatch& patchTemplate,
		size_t channel, size_t& numValidPatches) const
	{
		numValidPatches = 0;

		for (size_t p = 0; p < patches.size(); ++p)
		{
			if (patches[p].distance == std::numeric_limits<float>::max())
			{
				break;
			}
			for (size_t row = 0; row <patchTemplate.height; ++row)
			{
				for (size_t col = 0; col < patchTemplate.width; ++col)
				{
					size_t blockIdx = (patchTemplate.width * patchTemplate.height) * p + row * patchTemplate.width + col;
					block[blockIdx] = m_pixelData[channel][IDX2_2_1(patches[p].row + row, patches[p].col + col)];
				}
			}
			++numValidPatches;
		}
	}

	void Image::cpyfromBlock3d(const std::vector<IDX2>& patches, float* block, const ImagePatch& patchTemplate,
		size_t channel, size_t numValidPatches)
	{
		for (size_t p = 0; p < numValidPatches; ++p)
		{
			for (size_t row = 0; row < patchTemplate.height; ++row)
			{
				for (size_t col = 0; col < patchTemplate.width; ++col)
				{
					size_t blockIdx = (patchTemplate.width * patchTemplate.height) * p + row * patchTemplate.width + col;
					m_pixelData[channel][IDX2_2_1(patches[p].row + row, patches[p].col + col)] = block[blockIdx];
				}
			}
		}
	}

	void Image::print(int channel)
	{
		if (channel > 0)
		{
			for (int row = 0; row < m_actualImageDim.height; ++row)
			{
				for (int col = 0; col < m_actualImageDim.width; ++col)
				{
					if (col < m_actualImageDim.width - 1)
					{
						std::cout << m_pixelData[channel][IDX2_2_1(row, col)] << ", ";
					}
					else
					{
						std::cout << m_pixelData[channel][IDX2_2_1(row, col)] << std::endl;
					}
				}
				std::cout << std::endl;
			}
		}
		else
		{
			for (int c = 0; c < m_numChannels; ++c)
			{
				std::cout << "CHANNEL: " << c << std::endl << std::endl;
				for (int row = 0; row < m_actualImageDim.height; ++row)
				{
					for (int col = 0; col < m_actualImageDim.width; ++col)
					{
						if (col < m_actualImageDim.width - 1)
						{
							std::cout << m_pixelData[c][IDX2_2_1(row, col)] << ", ";
						}
						else
						{
							std::cout << m_pixelData[c][IDX2_2_1(row, col)] << std::endl;
						}
					}
				}
				std::cout << std::endl;
			}
		}

		std::cout << std::endl << std::endl;
	}

	void Image::setAlphaToOne()
	{
		switch (m_format)
		{
		case FLOAT_4:
			for (size_t i = 0; i < width(); ++i)
			{
				m_pixelData[3][i] = 1.0f;
			}
			break;
		default:
			printError("Cannot set Alpha Channel to one as format is not handled internally for this function.");
			break;
		}
	}
}