#pragma once
#include<vector>

namespace Denoise
{
	class Image;

	class ColourSpaceTransforms
	{
	public:
		ColourSpaceTransforms(Image* image);
		~ColourSpaceTransforms();

		void generateIdentityTransformMatrices();
		void generateOPPMatrices();
		void generateYUVMatrices();
		void generateYCbCrMatrices();

		void printTransformMatrices();

		std::vector<std::vector<float> >& getColourTransformMatrix() { return m_colourTransformMatrix; }
		std::vector<std::vector<float> >& getInverseColourTransformMatrix() { return m_colourTransformInvMatrix; }

		//! Transforms the input from RGB to the currently specified colour space
		void fromRGB();

		//! Transforms the input from the currently specified colour space to RGB
		void toRGB();

	private:
		void clearAll();
		std::vector<std::vector<float> > m_colourTransformMatrix;
		std::vector<std::vector<float> > m_colourTransformInvMatrix;

		Image* m_image;
	};
}


