#include "PNGLoader.h"

#include <iostream>

#include "lodepng.h"

PNGLoader::PNGLoader()
{
}


PNGLoader::~PNGLoader()
{
}

void PNGLoader::loadImage(Denoise::Image** image, const std::string& fileName)
{
	std::vector<unsigned char> png;
	std::vector<unsigned char> rawImage; //the raw pixels
	unsigned width, height;

	//load and decode
	lodepng::load_file(png, fileName);
	unsigned error = lodepng::decode(rawImage, width, height, png);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

	Denoise::Dimension dim(width, height);
	*image = new Denoise::Image(dim, Denoise::Image::FLOAT_4);

	for (index_t i = 0; i < rawImage.size(); i += 4)
	{
		for (index_t c = 0; c < 4; ++c)
		{
			(*image)->setPixel(c, i / 4, (float)rawImage[i + c]);
		}
	}
}

void PNGLoader::saveImage(Denoise::Image* image, const std::string& fileName)
{
	std::vector<unsigned char> png;

	std::vector<unsigned char> rawImage;
	rawImage.resize(image->width() * image->height() * 4);

	for (index_t row = 0; row < image->height(); ++row)
	{
		for (index_t col = 0; col < image->width(); ++col)
		{
			for (index_t c = 0; c < 4; ++c)
			{
				index_t i = (row * image->width() + col) * 4;

				if (image->getPixel(c, row, col) > 0)
				{
					rawImage[i + c] = (unsigned char)image->getPixel(c, row, col);
				}
				else
				{
					rawImage[i + c] = 0;
				}
			}
		}
	}

	unsigned error = lodepng::encode(png, rawImage, image->width(), image->height());
	if (!error) lodepng::save_file(png, fileName);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;

}