#pragma once

//OpenEXR
#include <ImfInputFile.h>
#include <ImfOutputFile.h>
#include <ImfChannelList.h>
#include <ImfArray.h>

namespace Denoise
{
	class EXRIO
	{
	public:
		static bool readEXR(const std::string& fileName,
			Imf::Array2D<float> &rPixels,
			Imf::Array2D<float> &gPixels,
			Imf::Array2D<float> &bPixels,
			Imf::Array2D<float> &aPixels,
			int &width, int &height,
			Imath::Box2i& dataWindow,
			Imath::Box2i& displayWindow);

		static void writeEXR(const std::string& fileName,
			Imf::Array2D<float> &rPixels,
			Imf::Array2D<float> &gPixels,
			Imf::Array2D<float> &bPixels,
			Imf::Array2D<float> &aPixels,
			Imath::Box2i& dataWindow,
			Imath::Box2i& displayWindow);


	private:
		EXRIO();
		~EXRIO();
	};
}




