// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
#define _WIN32_WINNT 0x0501

static const char* const CLASS = "Sakarnen";

static const char* const HELP =
"Non-Local Adaptive Image Denoiser";

// Standard plug-in include files.
#include "DDImage/Iop.h"
#include "DDImage/NukeWrapper.h"
using namespace DD::Image;
#include "DDImage/Row.h"
#include "DDImage/Tile.h"
#include "DDImage/Knobs.h"
#include "DDImage/Thread.h"

//MPC Denoiser Includes
#include "Image.h"
#include "BM3DImageBlockProcessor.h"
#include "BM3DSettings.h"

#include <iostream>
#include <vector>

using namespace std;

class Sakarnen : public Iop
{
private:
	float _maxValue;
	bool _firstTime;
	Lock _lock;

	Denoise::BM3DSettings m_settings;

	//knobs
	bool m_knobPreview;
	bool m_knobSmoothUniformRegions;
	double m_knobSmoothUniformRegionsFactor;
	bool m_knobAdaptivity;
	double m_knobAdaptivityScale;
	double m_knobAdaptivityPower;
	double m_knobSmoothness[3];
	int m_knobNumThreads;

	//preview box
	double m_knobPreviewBox[4];
	bool m_knobEnablePreviewBox;

public:

	int maximum_inputs() const { return 1; }
	int minimum_inputs() const { return 1; }

	//! Constructor. Initialize user controls to their default values.

	Sakarnen(Node* node) : Iop(node)
	{
		_maxValue = 0;
		_firstTime = true;
		m_knobPreview = true;
		
		m_knobSmoothUniformRegions = false;
		m_knobSmoothUniformRegionsFactor = 0.75;

		m_knobAdaptivity = false;
		m_knobAdaptivityScale = 0.25;
		m_knobAdaptivityPower = 3.0;

		m_knobNumThreads = 1;

		for (int i = 0; i < 3; ++i)
		{
			m_knobSmoothness[i] = 0.5;
		}

		//Preview Box
		m_knobEnablePreviewBox = true;
		m_knobPreviewBox[0] = 0.0;
		m_knobPreviewBox[0] = 0.0;
		m_knobPreviewBox[0] = 50.0;
		m_knobPreviewBox[0] = 50.0;

	}

	~Sakarnen()
	{

	}

	void _validate(bool);
	void _request(int x, int y, int r, int t, ChannelMask channels, int count);
	void _open();
	virtual void knobs(Knob_Callback);

	void engine(int y, int x, int r, ChannelMask channels, Row& out);

	void parseOptions(Denoise::BM3DSettings& settings);
	//! Return the name of the class.

	const char* Class() const { return CLASS; }
	const char* node_help() const { return HELP; }

	//! Information to the plug-in manager of DDNewImage/Nuke.

	static const Iop::Description description;

};


/*! This is a function that creates an instance of the operator, and is
needed for the Iop::Description to work.
*/
static Iop* SakarnenCreate(Node* node)
{
	std::cout << "Creating New Instance" << std::endl;
	return new Sakarnen(node);
}

/*! The Iop::Description is how NUKE knows what the name of the operator is,
how to create one, and the menu item to show the user. The menu item may be
0 if you do not want the operator to be visible.
*/
const Iop::Description Sakarnen::description(CLASS, "Sakarnen",
	SakarnenCreate);


void Sakarnen::_validate(bool for_real)
{
	copy_info(); // copy bbox channels etc from input0, which will validate it.
}

void Sakarnen::_request(int x, int y, int r, int t, ChannelMask channels, int count)
{
	// request all input input as we are going to search the whole input area
	ChannelSet readChannels = input0().info().channels();
	input(0)->request(readChannels, count);
}

void Sakarnen::_open()
{
	_firstTime = true;
}


/*! For each line in the area passed to request(), this will be called. It must
calculate the image data for a region at vertical position y, and between
horizontal positions x and r, and write it to the passed row
structure. Usually this works by asking the input for data, and modifying
it.

*/
void Sakarnen::engine(int y, int x, int r, ChannelMask channels, Row& row)
{
	{
    Guard guard(_lock);
      //1. BUILD Image Data struct for denoiser
      Format format = input0().format();

      const int fx = format.x(); //left edge
      const int fy = format.y(); //bottom edge
      const int fr = format.r(); //right edge
      const int ft = format.t(); //top edge

	  const int width = fr - fx;
	  const int height = ft - fy;

	  //create an empty image data struct
	  Denoise::Dimension dim(width, height);

	  Denoise::Image image(dim, Denoise::Image::FLOAT_4);
	  Denoise::Image basic(image);

	  Denoise::Image result(image);

	  if(_firstTime)
	  {
	  //get the channel set
      ChannelSet readChannels = input0().info().channels();

      Interest interest( input0(), fx, fy, fr, ft, readChannels, true );
      interest.unlock();
      
       //fetch each row and copy it the denoiser image data
      _maxValue = 0; 
      for ( int ry = fy; ry < ft; ry++)
	  {
			progressFraction( ry, ft - fy );
			Row row( fx, fr );
			row.get( input0(), ry, fx, fr, readChannels );
			//if ( aborted() )
			//{
			//	return;
			//}

			int channel = 0;
			foreach( z, readChannels )
			{
				const float *CUR = row[z] + fx;
				const float *END = row[z] + fr;
				int col = x + fx;
				while ( CUR < END )
				{
					if(channel == 0)
					{
						image.setPixel(0, ry, col, (float)*CUR);
					}
					else if(channel == 1)
					{
						image.setPixel(1, ry, col, (float)*CUR);
					}
					else if(channel == 2)
					{
						image.setPixel(2, ry, col, (float)*CUR);
						image.setPixel(3, ry, col, 1.0f);
					}
					//else if(channel == 3)
					//{
					//	m_image->setPixel(3, ry, col, (float)*CUR);
					//}

					CUR++;
					col++;
				}
				++channel;
			}
      }
	  }

	  //m_image->normalise();

	  //2. Run Denoiser
	  Denoise::BM3DImageBlockProcessor proc(&image, &basic, &result);

	  parseOptions(m_settings);
	  /*Denoise::BM3DSettings localSettings;
	  std::vector<float> temp;
	  temp.push_back(0.1f); temp.push_back(0.1f); temp.push_back(0.1f);
	  localSettings.init2defaults(temp, true);*/

	 // proc.process(m_settings, true);

	  //m_image->undoNormalise();
	  //m_basic->undoNormalise();
	  //m_result->undoNormalise();
	  _firstTime = false;
	  }
  
  Row in( x,r);
  in.get( input0(), y, x, r, channels );
  //if ( aborted() )
  //{
  //  return;
  //}
  int channel = 0;
  foreach( z, channels ) {
    float *CUR = row.writable(z) + x;
    const float* inptr = in[z] + x;
    const float *END = row[z] + r;
	int col = x;
    while ( CUR < END ) {
		//float value = (*m_denoisedImage.rPixels)[x][col];
		//*CUR++ = ((float) col) / 640.0f;
		//++col;
		//continue;

		if(channel == 0)
		{
			*CUR++ = basic.getPixel(0, y, col);
		}
		else if(channel == 1)
		{
			*CUR++ = basic.getPixel(1, y, col);
		}
		else if(channel == 2)
		{
			*CUR++ = basic.getPixel(2, y, col);
		}
		else if(channel == 3)
		{
			*CUR++ = 1.0f;
		}
		else
		{
			*CUR++ = -10.0f;
		}
		++col;
    }
	++channel;
  }
}

void Sakarnen::knobs(Knob_Callback f)
{
	Bool_knob(f, &m_knobPreview, "Preview");
	Tooltip(f, "Enables Preview-Quality Filtering.");

	Bool_knob(f, &m_knobAdaptivity, "Luminance-Adaptivity");
	Tooltip(f, "Enables luminance-adapative Filtering.");
	
	Double_knob(f, &m_knobAdaptivityScale, "Luminance-Adaptivity - Scale");
	Tooltip(f, "Scales the overall effect of this option.");

	Double_knob(f, &m_knobAdaptivityPower, "Luminance-Adaptivity Power");
	Tooltip(f, "Scales the difference between regions of difference brightness.");

	Bool_knob(f, &m_knobSmoothUniformRegions, "Uniform Region Smoothing");
	Tooltip(f, "Enables smart-smoothing of uniform areas to avoid artefacts.");

	Double_knob(f, &m_knobSmoothUniformRegionsFactor, "Uniform Region Smoothing - Scale");
	Tooltip(f, "Scales the overall effect of this option.");

	Int_knob(f, &m_knobNumThreads, "Number of Threads");
	Tooltip(f, "Number of threads used by the denoiser.");

	MultiFloat_knob(f, m_knobSmoothness, 3, "Smoothness for channels");
	Tooltip(f, "Smoothness values for each colour channel.");
}


void Sakarnen::parseOptions(Denoise::BM3DSettings& settings)
{
	std::vector<float> channelSmoothness(3);
	channelSmoothness[0] = m_knobSmoothness[0];
	channelSmoothness[1] = m_knobSmoothness[1];
	channelSmoothness[2] = m_knobSmoothness[2];

	settings.init2defaults(channelSmoothness, m_knobPreview);

	settings.stdDeviation = channelSmoothness;

	settings.limitHardwareConcurrency(m_knobNumThreads);

	if (m_knobPreview)
	{
		settings.previewQuality();
	}
	else
	{
		settings.productionQuality();
	}

	if (m_knobAdaptivity)
	{
		m_settings.enableMeanAdaptiveThresholding(m_knobAdaptivityScale, m_knobAdaptivityPower);
	}
	else
	{
		m_settings.meanAdaptiveThresholding = false;
	}

	if (m_knobSmoothUniformRegions)
	{
		m_settings.enableBlockStatisticalAveraging(m_knobSmoothUniformRegionsFactor);
	}
	else
	{
		m_settings.averageBlocksBasedOnStdCollaborative = false;
		m_settings.averageBlocksBasedOnStdWiener = false;
	}

}