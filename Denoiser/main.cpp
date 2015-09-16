#include <vector>
#include <string>
#include <iostream>

#include "lodepng.h"

#include "Image.h"
#include "ImageBlockProcessor.h"
#include "IDX2.h"
#include "ImagePatch.h"

void loadImage(Image** image, const std::string& fileName);
void createImage(Image** image);

void saveImage(Image* image, const std::string& fileName);


int main(int argc, char* argv[])
{
	std::string inputFile = "C:/Users/Stephan/Desktop/llama.png";
	std::string outputFile = "C:/Users/Stephan/Desktop/llama_padded.png";

	Image* image = nullptr;

	loadImage(&image, inputFile);
	image->checkImageIntegrity(true);
	image->normalise();
	/*Padding pad;
	pad.bottom = 10;
	pad.top = 20;
	pad.left = 20;
	pad.right = 10;
	image->padImage(pad, false);
	//image->accessFullImage();*/

	int idx = 10;

	std::vector<std::vector<IDX2> > similarPatches;
	std::vector<IDX2> similarPatchesComparison;
	ImagePatch templatePatch;
	templatePatch.height = 6;
	templatePatch.width = 6;
	templatePatch.col = 0;
	templatePatch.row = 0;
	IDX2 singlePosition(idx, idx);
	ImageBlockProcessor blockProcess(*image);

	//Image* image = nullptr;
	//createImage(&image);

	//std::cout << "Image: " << std::endl;
	//image->print();

	//std::vector<std::vector<IDX2> > similarPatches;
	//std::vector<IDX2> similarPatchesComparison;
	//ImagePatch templatePatch;
	//templatePatch.height = 2;
	//templatePatch.width = 2;
	//templatePatch.col = 0;
	//templatePatch.row = 0;
	//IDX2 singlePosition(2, 2);
	//ImageBlockProcessor blockProcess(*image);

	blockProcess.computeNMostSimilarNaive(similarPatchesComparison, singlePosition, templatePatch, 2, 2, 32, 10000.0f, 2);
	blockProcess.computeNMostSimilar(similarPatches, templatePatch, 1, 1, 2, 2, 32, 10000.0f, 2);

	std::cout << "Reference: " << std::endl;
	for (int i = 0; i < similarPatchesComparison.size(); ++i)
	{
		std::cout << "(" << similarPatchesComparison[i].col << ", " << similarPatchesComparison[i].row
			<< ", " << similarPatchesComparison[i].distance << ");  ";
	}

	std::cout << std::endl << "New: " << std::endl;
	for (int i = 0; i < similarPatches[image->width() * idx + idx].size(); ++i)
	{
		std::cout << "(" << similarPatches[image->width() * idx + idx][i].col << ", "
			<< similarPatches[image->width() * idx + idx][i].row << ", "
			<< similarPatches[image->width() * idx + idx][i].distance << ");  ";
	}

	//image->undoNormalise();
	//saveImage(image, outputFile);

	delete image;
}

void loadImage(Image** image, const std::string& fileName)
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

	Dimension dim(width, height);
	*image = new Image(dim, 4);

	for (size_t i = 0; i < rawImage.size(); i+=4)
	{
		for (size_t c = 0; c < 4; ++c)
		{
			(*image)->setPixel(c, i / 4, (float)rawImage[i + c]);
		}
	}
}

void saveImage(Image* image, const std::string& fileName)
{
	std::vector<unsigned char> png;

	std::vector<unsigned char> rawImage;
	rawImage.resize(image->width() * image->height() * 4);

	for (size_t row = 0; row < image->height(); ++row)
	{
		for (size_t col = 0; col < image->width(); ++col)
		{
			for (size_t c = 0; c < 4; ++c)
			{
				size_t i = (row * image->width() + col) * 4;

				rawImage[i + c] = (unsigned char)image->getPixel(c, row, col);
			}
		}
	}

	unsigned error = lodepng::encode(png, rawImage, image->width(), image->height());
	if (!error) lodepng::save_file(png, fileName);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;

}

void createImage(Image** image)
{
	Dimension dim(6, 6);
	*image = new Image(dim, 1);

	std::vector<float> values;
	values.push_back(1.0f);
	values.push_back(2.0f);
	values.push_back(3.0f);
	values.push_back(4.0f);
	values.push_back(5.0f);
	values.push_back(6.0f);

	values.push_back(6.0f);
	values.push_back(1.0f);
	values.push_back(2.0f);
	values.push_back(3.0f);
	values.push_back(5.0f);
	values.push_back(4.0f);

	values.push_back(5.0f);
	values.push_back(9.0f);
	values.push_back(7.0f);
	values.push_back(1.0f);
	values.push_back(2.0f);
	values.push_back(8.0f);

	values.push_back(6.0f);
	values.push_back(5.0f);
	values.push_back(1.0f);
	values.push_back(4.0f);
	values.push_back(2.0f);
	values.push_back(4.0f);

	values.push_back(9.0f);
	values.push_back(7.0f);
	values.push_back(8.0f);
	values.push_back(6.0f);
	values.push_back(5.0f);
	values.push_back(4.0f);

	values.push_back(4.0f);
	values.push_back(3.0f);
	values.push_back(6.0f);
	values.push_back(1.0f);
	values.push_back(2.0f);
	values.push_back(2.0f);

	values.push_back(1.0f);
	values.push_back(2.0f);
	values.push_back(1.0f);
	values.push_back(2.0f);
	values.push_back(2.0f);
	values.push_back(1.0f);

	for (size_t i = 0; i < 36; ++i)
	{
		(*image)->setPixel(0, i, values[i]);
	}
}