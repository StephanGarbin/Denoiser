#pragma once

#include<vector>
#include<string>

#include "ImagePatch.h"
#include "IDX2.h"
#include "Dimension.h"


namespace Denoise
{
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
		Image(const Dimension& imageDimension, size_t format);
		Image(const Image& other);
		~Image();

		inline float getPixel(const size_t channel, const size_t idx) const;
		inline float getPixel(const size_t channel, const size_t row, const size_t col) const;

		inline void setPixel(const size_t channel, const size_t idx, float value);
		inline void setPixel(const size_t channel, const size_t row, const size_t col, float value);

		bool padImage(Padding& padAmount, bool blackOutside);

		void accessFullImage();
		void accessActualImage();

		inline size_t width() const;
		inline size_t height() const;
		inline size_t size() const;
		inline size_t numChannels() const;
		inline Dimension fullDimension() const { return m_fullImageDim; }
		inline Dimension actualDimension() const { return m_actualImageDim; }

		inline size_t format() const { return m_format; }

		//Normalisation
		inline bool isNormalised() const { return m_isNormalised; }
		void normalise();
		void undoNormalise(float normalisationValue = -1.0f);

		inline float normalisationValue() { return m_normalisationValue; }

		//Block matching
		inline float blockMatch_Naive(const ImagePatch& patch1, const ImagePatch& patch2, size_t channel, int norm);

		//Block copy from/to
		void cpy2Block3d(const std::vector<IDX2>& patches, float* block, const ImagePatch& patchTemplate, size_t channel, size_t& numValidPatches) const;
		void cpyfromBlock3d(const std::vector<IDX2>& patches, float* block, const ImagePatch& patchTemplate, size_t channel, size_t numValidPatches);

		//Misceallaneous Functions
		float maxPixelValue(size_t channel) const;
		float minPixelValue(size_t channel) const;
		float averagePixelValue(size_t channel);

		bool checkImageIntegrity(bool enforceIntegrity);
		void print(int channel = -1) const;

		//Convenience
		void setAlphaToOne();

		inline size_t IDX2_2_1(const size_t row, const size_t col) const; //always uses full image

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
		size_t m_numChannels;

		bool m_accessingFullImage;
		bool m_isPadded;

		std::vector<float*> m_pixelData;

		size_t m_format;

		//Normalisation
		bool m_isNormalised;
		float m_normalisationValue;

		//General
		void initialise(const Dimension& imageDimension, size_t format);
		void printError(const std::string& message);
		void printWarning(const std::string& message);
		void printNotification(const std::string& message);

		int m_verbosityLevel;
	};

	float Image::getPixel(const size_t channel, const size_t idx) const
	{
		return m_pixelData[channel][idx];
	}

	float Image::getPixel(const size_t channel, const size_t row, const size_t col) const
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

	void Image::setPixel(const size_t channel, const size_t idx, float value)
	{
		m_pixelData[channel][idx] = value;
	}

	void Image::setPixel(const size_t channel, const size_t row, const size_t col, float value)
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

	size_t Image::width() const
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

	size_t Image::height() const
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

	size_t Image::size() const
	{
		return m_fullImageDim.height * m_fullImageDim.width;
	}

	size_t Image::numChannels() const
	{
		return m_numChannels;
	}

	size_t Image::IDX2_2_1(const size_t row, const size_t col) const
	{
		return row * m_fullImageDim.width + col;
	}

	float Image::blockMatch_Naive(const ImagePatch& patch1, const ImagePatch& patch2, size_t channel, int norm)
	{
		float sum = 0.0f;
		for (size_t i = 0; i < patch1.height; ++i)
		{
			for (size_t j = 0; j < patch1.width; ++j)
			{
				sum += std::pow(m_pixelData[channel][IDX2_2_1(patch1.row + i, patch1.col + j)]
					- m_pixelData[channel][IDX2_2_1(patch2.row + i, patch2.col + j)], norm);
			}
		}

		return sum;
	}
}