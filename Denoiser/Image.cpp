#include "Image.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <functional>
#include <math.h>

#include <boost/math/special_functions/fpclassify.hpp>

//OpenEXR
#include <ImfInputFile.h>
#include <ImfOutputFile.h>
#include <ImfChannelList.h>
#include <ImfArray.h>

#include "lodepng.h"
#include "EXRIO.h"

#include "common.h"

namespace Denoise
{
	struct IMFDATA
	{
		Imf::Array2D<float> rPixels;
		Imf::Array2D<float> gPixels;
		Imf::Array2D<float> bPixels;
		Imf::Array2D<float> aPixels;
		int width; int height;
		Imath::Box2i dataWindow;
		Imath::Box2i displayWindow;
	};

	Image::Image()
	{
		m_ImfData = new IMFDATA();
	}

	Image::Image(const Dimension& imageDimension, index_t format)
	{
		m_ImfData = new IMFDATA();
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
		for (index_t c = 0; c < m_pixelData.size(); ++c)
		{
			m_pixelData[c] = new float[m_fullImageDim.width * m_fullImageDim.height];
			for (index_t i = 0; i < m_fullImageDim.width * m_fullImageDim.height; ++i)
			{
				m_pixelData[c][i] = other.m_pixelData[c][i];
			}
		}

		m_colourSpaceTransform = new ColourSpaceTransforms(this);
		m_colourSpace = other.m_colourSpace;

		m_ImfData = new IMFDATA();
		m_ImfData->dataWindow = other.m_ImfData->dataWindow;
		m_ImfData->displayWindow = other.m_ImfData->displayWindow;
		m_ImfData->width = other.m_ImfData->width;
		m_ImfData->height = other.m_ImfData->height;
	}


	Image::~Image()
	{
		for (index_t c = 0; c < m_numChannels; ++c)
		{
			delete[] m_pixelData[c];
		}

		delete m_colourSpaceTransform;
		delete m_ImfData;
	}

	void Image::initialiseFromEmpty(const Dimension& imageDimension, index_t format)
	{
		initialise(imageDimension, format);
	}

	void Image::initialise(const Dimension& imageDimension, index_t format)
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
		for (index_t c = 0; c < m_pixelData.size(); ++c)
		{
			m_pixelData[c] = new float[imageDimension.width * imageDimension.height];
		}

		m_normalisationValue = 1.0f;
		m_colourSpaceTransform = new ColourSpaceTransforms(this);
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

		for (index_t c = 0; c < m_numChannels; ++c)
		{
			//Allocate new buffer;
			float* replacementChannel = new float[m_fullImageDim.width * m_fullImageDim.height];

			//Copy existing data
			for (index_t row = 0; row < m_actualImageDim.height; ++row)
			{
				for (index_t col = 0; col < m_actualImageDim.width; ++col)
				{
					index_t idxOriginal = row * m_actualImageDim.width + col;
					index_t idxPadded = (row + padAmount.top) * m_fullImageDim.width + (col + padAmount.left);
					replacementChannel[idxPadded] = m_pixelData[c][idxOriginal];
				}
			}

			//Swap pointers
			delete[] m_pixelData[c];
			m_pixelData[c] = replacementChannel;

			//Mirror pixel values if necessary
			index_t counter = padAmount.left;
			//left
			if (padAmount.left > 0)
			{
				for (index_t col = 0; col < padAmount.left; ++col)
				{
					for (index_t row = 0; row < m_fullImageDim.height; ++row)
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
				for (index_t row = padAmount.top - 1; row > 0; --row)
				{
					for (index_t col = 0; col < m_fullImageDim.width; ++col)
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
				for (index_t col = m_fullImageDim.width - padAmount.right; col < m_fullImageDim.width; ++col)
				{
					for (index_t row = 0; row < m_fullImageDim.height; ++row)
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
				for (index_t row = m_fullImageDim.height - padAmount.bottom; row < m_fullImageDim.height; ++row)
				{
					for (index_t col = 0; col < m_fullImageDim.width; ++col)
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
		for (index_t c = 0; c < maxPixelsValuesPerChannel.size(); ++c)
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
			for (index_t c = 0; c < m_pixelData.size(); ++c)
			{
				for (index_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
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

		for (index_t c = 0; c < m_pixelData.size(); ++c)
		{
			for (index_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
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

	float Image::maxPixelValue() const
	{
		float maxValue = std::numeric_limits<float>::min();
		for (index_t c = 0; c < m_numChannels; ++c)
		{
			float cMax = maxPixelValue(c);
			if (cMax > maxValue)
			{
				maxValue = cMax;
			}
		}

		return maxValue;
	}

	float Image::minPixelValue() const
	{
		float minValue = std::numeric_limits<float>::max();
		for (index_t c = 0; c < m_numChannels; ++c)
		{
			float cMin = minPixelValue(c);
			if (cMin < minValue)
			{
				minValue = cMin;
			}
		}
		return minValue;
	}

	float Image::maxPixelValue(index_t channel) const
	{
		float maxValue = std::numeric_limits<float>::min();
		for (index_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
		{
			maxValue = (m_pixelData[channel][i] > maxValue) ? m_pixelData[channel][i] : maxValue;
		}
		return maxValue;
	}

	float Image::minPixelValue(index_t channel) const
	{
		float minValue = std::numeric_limits<float>::max();
		for (index_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
		{
			minValue = (m_pixelData[channel][i] < minValue) ? m_pixelData[channel][i] : minValue;
		}
		return minValue;
	}

	float Image::averagePixelValue(index_t channel)
	{
		//do accumulation in double to avoid precision errors
		long double pixelSum = 0.0;
		for (index_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
		{
			pixelSum += (double)m_pixelData[channel][i];
		}
		pixelSum /= (double)(m_fullImageDim.height * m_fullImageDim.width);

		return (float)pixelSum;
	}

	bool Image::checkImageIntegrity(bool enforceIntegrity)
	{
		index_t containsNegative = 0;
		index_t containsNaNs = 0;
		index_t containsInfs = 0;

		for (index_t c = 0; c < m_pixelData.size(); ++c)
		{
			for (index_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
			{
				if (boost::math::isnan(m_pixelData[c][i]))
				{
					++containsNaNs;
				}
				else if (boost::math::isinf(m_pixelData[c][i]))
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
			for (index_t c = 0; c < m_pixelData.size(); ++c)
			{
				for (index_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
				{
					if (boost::math::isnan(m_pixelData[c][i]))
					{
						m_pixelData[c][i] = 0.0f;
					}
					else if (boost::math::isinf(m_pixelData[c][i]))
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
		int channel, index_t& numValidPatches) const
	{
		numValidPatches = 0;

		for (index_t p = 0; p < patches.size(); ++p)
		{
			if (patches[p].distance == std::numeric_limits<float>::max())
			{
				break;
			}

			++numValidPatches;
		}

		if (numValidPatches < 32)
		{
			if (numValidPatches >= 16)
			{
				numValidPatches = 16;
			}
			else
			{
				if (numValidPatches >= 8)
				{
					numValidPatches = 8;
				}
				else
				{
					if (numValidPatches >= 4)
					{
						numValidPatches = 4;
					}
					else
					{
						if (numValidPatches >= 2)
						{
							numValidPatches = 2;
						}
						else
						{
							if (numValidPatches == 1)
							{
								numValidPatches = 1;
							}
							else
							{
								numValidPatches = 0;
							}
						}
					}
				}
			}
		}
		else
		{
			numValidPatches = 32;
		}

		if (channel < 0)
		{
			for (index_t c = 0; c < channel * -1; ++c)
			{
				for (index_t p = 0; p < numValidPatches; ++p)
				{
					for (index_t row = 0; row < patchTemplate.height; ++row)
					{
						for (index_t col = 0; col < patchTemplate.width; ++col)
						{
							index_t valuesPerChannel = (patchTemplate.width * patchTemplate.height) * numValidPatches;
							index_t blockIdx = c * valuesPerChannel + (patchTemplate.width * patchTemplate.height) * p + row * patchTemplate.width + col;
							index_t imageIdx = IDX2_2_1(patches[p].row + row, patches[p].col + col);
							//if (imageIdx > m_fullImageDim.height * m_fullImageDim.height)
							//{
							//	std::cout << "Patch Error; row = " << patches[p].row << ", col = " << patches[p].col << std::endl;
							//	continue;
							//}
							block[blockIdx] = m_pixelData[c][imageIdx];
						}
					}
				}
			}
		}
		else
		{
			for (index_t p = 0; p < numValidPatches; ++p)
			{
				for (index_t row = 0; row < patchTemplate.height; ++row)
				{
					for (index_t col = 0; col < patchTemplate.width; ++col)
					{
						index_t blockIdx = (patchTemplate.width * patchTemplate.height) * p + row * patchTemplate.width + col;
						block[blockIdx] = m_pixelData[channel][IDX2_2_1(patches[p].row + row, patches[p].col + col)];
					}
				}
			}
		}
	}

	void Image::cpy2Block3d(const std::vector<IDX2>& patches, double* block, const ImagePatch& patchTemplate,
		int channel, index_t& numValidPatches) const
	{
		numValidPatches = 0;

		for (index_t p = 0; p < patches.size(); ++p)
		{
			if (patches[p].distance == std::numeric_limits<float>::max())
			{
				break;
			}

			++numValidPatches;
		}

		if (numValidPatches < 32)
		{
			if (numValidPatches >= 16)
			{
				numValidPatches = 16;
			}
			else
			{
				if (numValidPatches >= 8)
				{
					numValidPatches = 8;
				}
				else
				{
					if (numValidPatches >= 4)
					{
						numValidPatches = 4;
					}
					else
					{
						if (numValidPatches >= 2)
						{
							numValidPatches = 2;
						}
						else
						{
							if (numValidPatches == 1)
							{
								numValidPatches = 1;
							}
							else
							{
								numValidPatches = 0;
							}
						}
					}
				}
			}
		}
		else
		{
			numValidPatches = 32;
		}

		if (channel < 0)
		{
			for (index_t c = 0; c < channel * -1; ++c)
			{
				for (index_t p = 0; p < numValidPatches; ++p)
				{
					for (index_t row = 0; row < patchTemplate.height; ++row)
					{
						for (index_t col = 0; col < patchTemplate.width; ++col)
						{
							index_t valuesPerChannel = (patchTemplate.width * patchTemplate.height) * numValidPatches;
							index_t blockIdx = c * valuesPerChannel + (patchTemplate.width * patchTemplate.height) * p + row * patchTemplate.width + col;
							index_t imageIdx = IDX2_2_1(patches[p].row + row, patches[p].col + col);
							//if (imageIdx > m_fullImageDim.height * m_fullImageDim.height)
							//{
							//	std::cout << "Patch Error; row = " << patches[p].row << ", col = " << patches[p].col << std::endl;
							//	continue;
							//}
							block[blockIdx] = (double)m_pixelData[c][imageIdx];
						}
					}
				}
			}
		}
		else
		{
			for (index_t p = 0; p < numValidPatches; ++p)
			{
				for (index_t row = 0; row < patchTemplate.height; ++row)
				{
					for (index_t col = 0; col < patchTemplate.width; ++col)
					{
						index_t blockIdx = (patchTemplate.width * patchTemplate.height) * p + row * patchTemplate.width + col;
						block[blockIdx] = (double)m_pixelData[channel][IDX2_2_1(patches[p].row + row, patches[p].col + col)];
					}
				}
			}
		}
	}


	void Image::cpyfromBlock3d(const std::vector<IDX2>& patches, float* block, const ImagePatch& patchTemplate,
		index_t channel, index_t numValidPatches)
	{
		for (index_t p = 0; p < numValidPatches; ++p)
		{
			for (index_t row = 0; row < patchTemplate.height; ++row)
			{
				for (index_t col = 0; col < patchTemplate.width; ++col)
				{
					index_t blockIdx = (patchTemplate.width * patchTemplate.height) * p + row * patchTemplate.width + col;
					m_pixelData[channel][IDX2_2_1(patches[p].row + row, patches[p].col + col)] = block[blockIdx];
				}
			}
		}
	}

	void Image::print(int channel) const
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
			for (index_t i = 0; i < width(); ++i)
			{
				m_pixelData[3][i] = 1.0f;
			}
			break;
		default:
			printError("Cannot set Alpha Channel to one as format is not handled internally for this function.");
			break;
		}
	}

	void Image::clamp(float minValue, float maxValue)
	{
		for (index_t c = 0; c < m_pixelData.size(); ++c)
		{
			for (index_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
			{
				if (m_pixelData[c][i] < minValue)
				{
					m_pixelData[c][i] = minValue;
				}

				if (m_pixelData[c][i] > maxValue)
				{
					m_pixelData[c][i] = maxValue;
				}
			}
		}
	}

	float Image::pixelMean(index_t channel, bool ignoreZeroPixelValues)
	{
		index_t counter = 0;
		long double accumulator = 0.0;
		for (index_t i = 0; i < m_fullImageDim.height * m_fullImageDim.width; ++i)
		{
			if (ignoreZeroPixelValues)
			{
				if (m_pixelData[channel][i] != 0.0f)
				{
					accumulator += m_pixelData[channel][i];
					++counter;
				}
			}
			else
			{
				accumulator += m_pixelData[channel][i];
				++counter;
			}
				
		}

		return (float)(accumulator / (long double)counter);
	}

	void Image::generateColourSpaceTransformationMatrices(index_t space)
	{
		switch (space)
		{
		case RGB:
			m_colourSpaceTransform->generateIdentityTransformMatrices();
			break;
		case YUV:
			m_colourSpaceTransform->generateYUVMatrices();
			break;
		case YCBCR:
			m_colourSpaceTransform->generateYCbCrMatrices();
			break;
		case OPP:
			m_colourSpaceTransform->generateOPPMatrices();
			break;
		default:
			printError("Colour Space Transform Initialisation failed - unknown argument!");
			break;
		}
	}

	void Image::toColourSpace(index_t space)
	{
		switch (space)
		{
			case RGB:
				if (m_colourSpace == RGB)
				{
					printNotification("Image already in RGB - transform ignored.");
				}
				else
				{
					generateColourSpaceTransformationMatrices(m_colourSpace);
					m_colourSpaceTransform->toRGB();
					m_colourSpace = RGB;
				}
				break;
			case YUV:
				if (m_colourSpace != RGB)
				{
					printError("Image not in RGB. Please transform to RGB before attempting a transform.");
				}
				else
				{
					generateColourSpaceTransformationMatrices(space);
					m_colourSpaceTransform->fromRGB();
					m_colourSpace = YUV;
				}
				break;
			case YCBCR:
				if (m_colourSpace != RGB)
				{
					printError("Image not in RGB. Please transform to RGB before attempting a transform.");
				}
				else
				{
					generateColourSpaceTransformationMatrices(space);
					m_colourSpaceTransform->fromRGB();
					m_colourSpace = YCBCR;
				}
				break;
			case OPP:
				if (m_colourSpace != RGB)
				{
					printError("Image not in RGB. Please transform to RGB before attempting a transform.");
				}
				else
				{
					generateColourSpaceTransformationMatrices(space);
					m_colourSpaceTransform->fromRGB();
					m_colourSpace = OPP;
				}
				break;
		default:
			printError("Colour Space Transform failed - unknown argument!");
			break;
		}
	}

	bool Image::readFromPNG(const std::string& file)
	{
		std::vector<unsigned char> png;
		std::vector<unsigned char> rawImage; //the raw pixels
		unsigned width, height;

		//load and decode
		lodepng::load_file(png, file);
		unsigned error = lodepng::decode(rawImage, width, height, png);

		//Handle errors
		if (error)
		{
			std::cout << "Decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
			std::cout << "Could not read [ " << file << " ].";
			return false;
		}
		

		Denoise::Dimension dim(width, height);
		
		initialise(dim, Denoise::Image::FLOAT_4);

		for (index_t i = 0; i < rawImage.size(); i += 4)
		{
			for (index_t c = 0; c < 4; ++c)
			{
				setPixel(c, i / 4, (float)rawImage[i + c]);
			}
		}

		return true;
	}
	
	bool Image::save2PNG(const std::string& file)
	{
		std::vector<unsigned char> png;

		std::vector<unsigned char> rawImage;
		rawImage.resize(width() * height() * 4);

		for (index_t row = 0; row < height(); ++row)
		{
			for (index_t col = 0; col < width(); ++col)
			{
				for (index_t c = 0; c < 4; ++c)
				{
					index_t i = (row * width() + col) * 4;

					if (getPixel(c, row, col) > 0)
					{
						rawImage[i + c] = (unsigned char)getPixel(c, row, col);
					}
					else
					{
						rawImage[i + c] = 0;
					}
				}
			}
		}

		unsigned error = lodepng::encode(png, rawImage, width(), height());

		if (!error)
		{
			lodepng::save_file(png, file);
		}
		else
		{
			std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
			std::cout << "Could not write [ " << file << " ].";
			return false;
		}

		return true;
	}

	bool Image::readFromEXR(const std::string& file)
	{
		EXRIO::readEXR(file, m_ImfData->rPixels, m_ImfData->gPixels,
			m_ImfData->bPixels, m_ImfData->aPixels, m_ImfData->width,
			m_ImfData->height, m_ImfData->dataWindow,
			m_ImfData->displayWindow);

		//Now transfer pixel values to the image
		Denoise::Dimension dim(m_ImfData->width, m_ImfData->height);

		initialise(dim, Denoise::Image::FLOAT_4);

		for (index_t row = 0; row < m_ImfData->height; ++row)
		{
			for (index_t col = 0; col < m_ImfData->width; ++col)
			{
				setPixel(0, row, col, m_ImfData->rPixels[row][col]);
				setPixel(1, row, col, m_ImfData->gPixels[row][col]);
				setPixel(2, row, col, m_ImfData->bPixels[row][col]);
				setPixel(3, row, col, m_ImfData->aPixels[row][col]);
			}
		}

		m_ImfData->rPixels.resizeErase(0, 0);
		m_ImfData->gPixels.resizeErase(0, 0);
		m_ImfData->bPixels.resizeErase(0, 0);
		m_ImfData->aPixels.resizeErase(0, 0);

		return true;
	}
	
	bool Image::save2EXR(const std::string& file)
	{
		m_ImfData->rPixels.resizeErase(height(), width());
		m_ImfData->gPixels.resizeErase(height(), width());
		m_ImfData->bPixels.resizeErase(height(), width());
		m_ImfData->aPixels.resizeErase(height(), width());

		for (index_t row = 0; row < height(); ++row)
		{
			for (index_t col = 0; col < width(); ++col)
			{
				m_ImfData->rPixels[row][col] = getPixel(0, row, col);
				m_ImfData->gPixels[row][col] = getPixel(1, row, col);
				m_ImfData->bPixels[row][col] = getPixel(2, row, col);
				m_ImfData->aPixels[row][col] = getPixel(3, row, col);
				//std::cout << getPixel(0, row, col) << " ";
			}
			//std::cout << std::endl;
		}

		EXRIO::writeEXR(file, m_ImfData->rPixels, m_ImfData->gPixels,
			m_ImfData->bPixels, m_ImfData->aPixels, m_ImfData->dataWindow,
			m_ImfData->displayWindow);

		
		return true;
	}

	bool Image::readFromFile(const std::string& file)
	{
		std::string fExtension;

		fExtension = file.substr(file.find_last_of(".") + 1);

		if (fExtension == "png")
		{
			return readFromPNG(file);
		}
		else if (fExtension == "exr")
		{
			return readFromEXR(file);
		}
		else
		{
			std::cout << "ERROR: Unrecognised File format [ " << fExtension << " ]" << std::endl;
			return false;
		}
	}

	bool Image::save2File(const std::string& file)
	{
		std::string fExtension;

		fExtension = file.substr(file.find_last_of(".") + 1);

		if (fExtension == "png")
		{
			save2PNG(file);
		}
		else if (fExtension == "exr")
		{
			save2EXR(file);
		}
		else
		{
			std::cout << "ERROR: Unrecognised File format [ " << fExtension << " ]" << std::endl;
			return false;
		}
	}
}