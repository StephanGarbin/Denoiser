#pragma once

#include <string>

#include <Denoiser\Image.h>

class PNGLoader
{
public:
	PNGLoader();
	~PNGLoader();

	void loadImage(Denoise::Image** image, const std::string& fileName);

	void saveImage(Denoise::Image* image, const std::string& fileName);
};

