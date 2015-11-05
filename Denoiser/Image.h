#pragma once

#include<vector>
#include<string>

#include "ImagePatch.h"
#include "IDX2.h"
#include "Dimension.h"
#include "common.h"

namespace Denoise
{
	struct Padding
	{
		index_t left;
		index_t right;
		index_t top;
		index_t bottom;
	};

	class Image
	{
	public:
		Image(const Dimension& imageDimension, index_t format);
		Image(const Image& other);
		~Image();

		inline float getPixel(const index_t channel, const index_t idx) const;
		inline float getPixel(const index_t channel, const index_t row, const index_t col) const;

		inline void setPixel(const index_t channel, const index_t idx, float value);
		inline void setPixel(const index_t channel, const index_t row, const index_t col, float value);

		bool padImage(Padding& padAmount, bool blackOutside);

		void accessFullImage();
		void accessActualImage();

		inline index_t width() const;
		inline index_t height() const;
		inline index_t size() const;
		inline index_t numChannels() const;
		inline Dimension fullDimension() const { return m_fullImageDim; }
		inline Dimension actualDimension() const { return m_actualImageDim; }

		inline index_t format() const { return m_format; }

		//Normalisation
		inline bool isNormalised() const { return m_isNormalised; }
		void normalise();
		void undoNormalise(float normalisationValue = -1.0f);

		inline float normalisationValue() { return m_normalisationValue; }

		//Block matching
		inline float blockMatch_Naive(const ImagePatch& patch1, const ImagePatch& patch2, index_t channel, int norm);

		//Block copy from/to
		void cpy2Block3d(const std::vector<IDX2>& patches, float* block, const ImagePatch& patchTemplate, int channel, index_t& numValidPatches) const;
		void cpyfromBlock3d(const std::vector<IDX2>& patches, float* block, const ImagePatch& patchTemplate, index_t channel, index_t numValidPatches);

		//Statistics
		float pixelMean(index_t channel, bool ignoreZeroPixelValues);

		//Misceallaneous Functions
		float maxPixelValue() const;
		float minPixelValue() const;

		float maxPixelValue(index_t channel) const;
		float minPixelValue(index_t channel) const;
		float averagePixelValue(index_t channel);

		void clamp(float minValue, float maxValue);

		bool checkImageIntegrity(bool enforceIntegrity);
		void print(int channel = -1) const;

		//Convenience
		void setAlphaToOne();

		inline index_t IDX2_2_1(const index_t row, const index_t col) const; //always uses full image

		enum Channels
		{
			R,
			G,
			B,
			A,
			ALL
		};

		enum Format
		{
			FLOAT_1,
			FLOAT_2,
			FLOAT_3,
			FLOAT_4
		};

	private:
		//Main Pixel Data
		Dimension m_fullImageDim;
		Dimension m_actualImageDim;
		Padding m_padding;
		index_t m_numChannels;

		bool m_accessingFullImage;
		bool m_isPadded;

		std::vector<float*> m_pixelData;

		index_t m_format;

		//Normalisation
		bool m_isNormalised;
		float m_normalisationValue;

		//General
		void initialise(const Dimension& imageDimension, index_t format);
		void printError(const std::string& message);
		void printWarning(const std::string& message);
		void printNotification(const std::string& message);

		int m_verbosityLevel;
	};

	float Image::getPixel(const index_t channel, const index_t idx) const
	{
		return m_pixelData[channel][idx];
	}

	float Image::getPixel(const index_t channel, const index_t row, const index_t col) const
	{
		if (m_isPadded && !m_accessingFullImage)
		{
			return m_pixelData[channel][IDX2_2_1(row + m_padding.top, col + m_padding.left)];
		}
		else
		{
			return m_pixelData[channel][IDX2_2_1(row, col)];
		}
	}

	void Image::setPixel(const index_t channel, const index_t idx, float value)
	{
		m_pixelData[channel][idx] = value;
	}

	void Image::setPixel(const index_t channel, const index_t row, const index_t col, float value)
	{
		if (m_isPadded && !m_accessingFullImage)
		{
			m_pixelData[channel][IDX2_2_1(row + m_padding.top, col + m_padding.left)] = value;
		}
		else
		{
			m_pixelData[channel][IDX2_2_1(row, col)] = value;
		}
	}

	index_t Image::width() const
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

	index_t Image::height() const
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

	index_t Image::size() const
	{
		return m_fullImageDim.height * m_fullImageDim.width;
	}

	index_t Image::numChannels() const
	{
		return m_numChannels;
	}

	index_t Image::IDX2_2_1(const index_t row, const index_t col) const
	{
		return row * m_fullImageDim.width + col;
	}

	float Image::blockMatch_Naive(const ImagePatch& patch1, const ImagePatch& patch2, index_t channel, int norm)
	{
		float sum = 0.0f;
		for (index_t i = 0; i < patch1.height; ++i)
		{
			for (index_t j = 0; j < patch1.width; ++j)
			{
				sum += std::pow(m_pixelData[channel][IDX2_2_1(patch1.row + i, patch1.col + j)]
					- m_pixelData[channel][IDX2_2_1(patch2.row + i, patch2.col + j)], norm);
			}
		}

		return sum;
	}
}