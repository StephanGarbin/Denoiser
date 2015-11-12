#include "ColourSpaceTransforms.h"

#include <iostream>

#include "Image.h"

namespace Denoise
{

	ColourSpaceTransforms::ColourSpaceTransforms(Image* image) : m_image(image)
	{
		generateIdentityTransformMatrices();
	}


	ColourSpaceTransforms::~ColourSpaceTransforms()
	{
		clearAll();
	}

	void
		ColourSpaceTransforms::clearAll()
	{
			m_colourTransformMatrix.clear();
			m_colourTransformInvMatrix.clear();
		}

	void
		ColourSpaceTransforms::generateIdentityTransformMatrices()
	{
			clearAll();
			//Transformation (Non-normalised!)
			std::vector<float> temp;
			temp.push_back(1.0);
			temp.push_back(0.0);
			temp.push_back(0.0);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			temp.push_back(0.0);
			temp.push_back(1.0);
			temp.push_back(0.0);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			temp.push_back(0.0);
			temp.push_back(0.0);
			temp.push_back(1.0);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			//Inverse Transformation (non-normalised)
			temp.push_back(1.0);
			temp.push_back(0.0);
			temp.push_back(0.0);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();

			temp.push_back(0.0);
			temp.push_back(1.0);
			temp.push_back(0.0);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();

			temp.push_back(0.0);
			temp.push_back(0.0);
			temp.push_back(1.0);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();
		}

	void
		ColourSpaceTransforms::generateOPPMatrices()
	{
			clearAll();
			//Transformation (Non-normalised!)
			std::vector<float> temp;
			temp.push_back(1.0 / 3.0);
			temp.push_back(1.0 / 3.0);
			temp.push_back(1.0 / 3.0);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			temp.push_back(1.0 / 2.0);
			temp.push_back(0.0);
			temp.push_back(-1.0 / 2.0);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			temp.push_back(1.0 / 4.0);
			temp.push_back(-1.0 / 2.0);
			temp.push_back(1.0 / 4.0);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			//Inverse Transformation (non-normalised)
			temp.push_back(1.0);
			temp.push_back(1.0);
			temp.push_back(2.0 / 3.0);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();

			temp.push_back(1.0);
			temp.push_back(0.0);
			temp.push_back(-1.0 - 1.0 / 3.0);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();

			temp.push_back(1.0);
			temp.push_back(-1.0);
			temp.push_back(2.0 / 3.0);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();
		}

	void
		ColourSpaceTransforms::generateYUVMatrices()
	{
			clearAll();
			//Transformation (Non-normalised!)
			std::vector<float> temp;
			temp.push_back(0.3);
			temp.push_back(0.59);
			temp.push_back(0.11);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			temp.push_back(-0.15);
			temp.push_back(-0.29);
			temp.push_back(0.44);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			temp.push_back(0.61);
			temp.push_back(-0.51);
			temp.push_back(-0.10);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			//Inverse Transformation (non-normalised)
			temp.push_back(1.0);
			temp.push_back(0.011444356748224);
			temp.push_back(1.150355169692186);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();

			temp.push_back(1.0);
			temp.push_back(-0.383188634569850);
			temp.push_back(-0.586029992107340);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();

			temp.push_back(1.0);
			temp.push_back(2.024072612470402);
			temp.push_back(0.005919494869771);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();
		}

	void
		ColourSpaceTransforms::generateYCbCrMatrices()
	{
			clearAll();
			//Transformation (Non-normalised!)
			std::vector<float> temp;
			temp.push_back(0.3);
			temp.push_back(0.59);
			temp.push_back(0.11);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			temp.push_back(-0.17);
			temp.push_back(-0.33);
			temp.push_back(0.5);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			temp.push_back(0.5);
			temp.push_back(-0.42);
			temp.push_back(-0.08);
			m_colourTransformMatrix.push_back(temp);
			temp.clear();

			//Inverse Transformation (non-normalised)
			temp.push_back(1.0);
			temp.push_back(0.004230118443316);
			temp.push_back(1.401438240270728);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();

			temp.push_back(1.0);
			temp.push_back(-0.334179357021997);
			temp.push_back(-0.713620981387479);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();

			temp.push_back(1.0);
			temp.push_back(1.7808798646362100);
			temp.push_back(0.005499153976311);
			m_colourTransformInvMatrix.push_back(temp);
			temp.clear();
		}

	void
		ColourSpaceTransforms::printTransformMatrices()
	{
			std::cout << "Transform Matrix:" << std::endl;

			for (int row = 0; row < 3; ++row)
			{
				std::cout << m_colourTransformMatrix[row][0] << ", "
					<< m_colourTransformMatrix[row][1] << ", "
					<< m_colourTransformMatrix[row][2] << std::endl;
			}

			std::cout << std::endl << "Inverse Transform Matrix:" << std::endl;

			for (int row = 0; row < 3; ++row)
			{
				std::cout << m_colourTransformInvMatrix[row][0] << ", "
					<< m_colourTransformInvMatrix[row][1] << ", "
					<< m_colourTransformInvMatrix[row][2] << std::endl;
			}

			std::cout << std::endl << std::endl;
		}


	void
		ColourSpaceTransforms::fromRGB()
	{
			float* temp = new float[3];

			for (int row = 0; row < m_image->height(); ++row)
			{
				for (int col = 0; col < m_image->width(); ++col)
				{
					for (int c = 0; c < 3; ++c)
					{
						temp[c] = m_image->getPixel(0, row, col) * m_colourTransformMatrix[c][0]
							+ m_image->getPixel(1, row, col) * m_colourTransformMatrix[c][1]
							+ m_image->getPixel(2, row, col) * m_colourTransformMatrix[c][2];
					}

					m_image->setPixel(0, row, col, temp[0]);
					m_image->setPixel(1, row, col, temp[1]);
					m_image->setPixel(2, row, col, temp[2]);
				}
			}

			delete[] temp;
		}


	void
		ColourSpaceTransforms::toRGB()
	{
			float* temp = new float[3];

			for (int row = 0; row < m_image->height(); ++row)
			{
				for (int col = 0; col < m_image->width(); ++col)
				{
					for (int c = 0; c < 3; ++c)
					{
						temp[c] = m_image->getPixel(0, row, col) * m_colourTransformInvMatrix[c][0]
							+ m_image->getPixel(1, row, col) * m_colourTransformInvMatrix[c][1]
							+ m_image->getPixel(2, row, col) * m_colourTransformInvMatrix[c][2];
					}

					m_image->setPixel(0, row, col, temp[0]);
					m_image->setPixel(1, row, col, temp[1]);
					m_image->setPixel(2, row, col, temp[2]);
				}
			}

			delete[] temp;
		}

}

