#include "EXRIO.h"

namespace Denoise
{

	EXRIO::EXRIO()
	{
	}


	EXRIO::~EXRIO()
	{
	}

	bool
		EXRIO::readEXR(const std::string& fileName,
		Imf::Array2D<float> &rPixels,
		Imf::Array2D<float> &gPixels,
		Imf::Array2D<float> &bPixels,
		Imf::Array2D<float> &aPixels,
		int &width, int &height,
		Imath::Box2i& dataWindow,
		Imath::Box2i& displayWindow)
	{
			Imf::InputFile file(fileName.c_str());
			displayWindow = file.header().displayWindow();
			dataWindow = file.header().dataWindow();

			width = dataWindow.max.x - dataWindow.min.x + 1;
			height = dataWindow.max.y - dataWindow.min.y + 1;

			//Make sure that we can handle empty images correctly
			if (width * height < 1)
			{
				return false;
			}

			rPixels.resizeErase(height, width);
			gPixels.resizeErase(height, width);
			bPixels.resizeErase(height, width);
			aPixels.resizeErase(height, width);

			Imf::FrameBuffer frameBuffer;

			frameBuffer.insert("R",
				Imf::Slice(Imf::FLOAT,
				(char *)(&rPixels[0][0] -
				dataWindow.min.x -
				dataWindow.min.y * width),
				sizeof (rPixels[0][0]) * 1,
				sizeof (rPixels[0][0]) * width,
				1, 1,
				0.0));

			frameBuffer.insert("G",
				Imf::Slice(Imf::FLOAT,
				(char *)(&gPixels[0][0] -
				dataWindow.min.x -
				dataWindow.min.y * width),
				sizeof (gPixels[0][0]) * 1,
				sizeof (gPixels[0][0]) * width,
				1, 1,
				0.0));

			frameBuffer.insert("B",
				Imf::Slice(Imf::FLOAT,
				(char *)(&bPixels[0][0] -
				dataWindow.min.x -
				dataWindow.min.y * width),
				sizeof (bPixels[0][0]) * 1,
				sizeof (bPixels[0][0]) * width,
				1, 1,
				0.0));

			frameBuffer.insert("A",
				Imf::Slice(Imf::FLOAT,
				(char *)(&aPixels[0][0] -
				dataWindow.min.x -
				dataWindow.min.y * width),
				sizeof (aPixels[0][0]) * 1,
				sizeof (aPixels[0][0]) * width,
				1, 1,
				0.0));

			file.setFrameBuffer(frameBuffer);
			file.readPixels(dataWindow.min.y, dataWindow.max.y);

			return true;
		}


	void
		EXRIO::writeEXR(const std::string& fileName,
		Imf::Array2D<float> &rPixels,
		Imf::Array2D<float> &gPixels,
		Imf::Array2D<float> &bPixels,
		Imf::Array2D<float> &aPixels,
		Imath::Box2i& dataWindow,
		Imath::Box2i& displayWindow)
	{
			try
			{
				Imf::Header header(displayWindow, dataWindow);

				header.channels().insert("R", Imf::Channel(Imf::FLOAT));
				header.channels().insert("G", Imf::Channel(Imf::FLOAT));
				header.channels().insert("B", Imf::Channel(Imf::FLOAT));
				header.channels().insert("A", Imf::Channel(Imf::FLOAT));


				Imf::OutputFile file(fileName.c_str(), header);
				Imf::FrameBuffer frameBuffer;

				int dwWidth = dataWindow.max.x - dataWindow.min.x + 1;

				frameBuffer.insert("R",
					Imf::Slice(Imf::FLOAT,
					(char *)(&rPixels[0][0] - dataWindow.min.x - dataWindow.min.y * dwWidth),
					sizeof (rPixels[0][0]) * 1,
					sizeof (rPixels[0][0]) * dwWidth));

				frameBuffer.insert("G",
					Imf::Slice(Imf::FLOAT,
					(char *)(&gPixels[0][0] - dataWindow.min.x - dataWindow.min.y * dwWidth),
					sizeof (gPixels[0][0]) * 1,
					sizeof (gPixels[0][0]) * dwWidth));

				frameBuffer.insert("B",
					Imf::Slice(Imf::FLOAT,
					(char *)(&bPixels[0][0] - dataWindow.min.x - dataWindow.min.y * dwWidth),
					sizeof (bPixels[0][0]) * 1,
					sizeof (bPixels[0][0]) * dwWidth));

				frameBuffer.insert("A",
					Imf::Slice(Imf::FLOAT,
					(char *)(&aPixels[0][0] - dataWindow.min.x - dataWindow.min.y * dwWidth),
					sizeof (aPixels[0][0]) * 1,
					sizeof (aPixels[0][0]) * dwWidth));

				file.setFrameBuffer(frameBuffer);
				file.writePixels(header.dataWindow().max.y - header.dataWindow().min.y + 1);
			}
			catch (Iex::BaseExc & e)
			{
				std::cerr << e.what() << std::endl;
			}
		}

}

