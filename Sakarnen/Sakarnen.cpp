#include "sakarnen.h"

#include <qfiledialog.h>

#include "PNGLoader.h"
#include "Denoiser\Image.h"

struct SakarnenData
{
	Denoise::Image* noisyImage;
	Denoise::Image* basicImage;
	Denoise::Image* resultImage;
};


Sakarnen::Sakarnen(QWidget *parent)
: QMainWindow(parent)
{
	setupGeneral();
	m_viewport = new Viewport2D(this);
	this->setCentralWidget(m_viewport);

	m_settingsWidget = new SettingsWidgetRight(this);
	this->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_settingsWidget);

	//Create Menus
	setupMenus();
}

void Sakarnen::setupGeneral()
{
	m_data = std::make_shared<SakarnenData>();
}

void Sakarnen::setupMenus()
{
	m_actionLoadImage = new QAction(tr("&Load Image"), this);
	m_actionLoadImage->setStatusTip("Load an Image from disk");
	connect(m_actionLoadImage, SIGNAL(triggered()), this, SLOT(loadImage()));

	m_actionSaveImage = new QAction(tr("&Save Image"), this);
	m_actionSaveImage->setStatusTip("Save an Image to disk");
	connect(m_actionSaveImage, SIGNAL(triggered()), this, SLOT(saveImage()));

	m_fileMenu = menuBar()->addMenu(tr("&File"));
	m_fileMenu->addAction(m_actionLoadImage);
	m_fileMenu->addAction(m_actionSaveImage);
}

void Sakarnen::loadImage()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Load Image"),
		tr(""),
		tr("EXR Files (*.exr);;"
		"PNG Files (*.png)"));

	if (fileName.isEmpty())
	{
		statusBar()->showMessage("ERROR: File could not found.");
	}
	else
	{
		//1. Clear images if necessary
		if (m_data->noisyImage == NULL)
		{
			delete m_data->noisyImage;
			delete m_data->basicImage;
			delete m_data->resultImage;
		}

		//2. Load new images
		PNGLoader loader;
		loader.loadImage(&m_data->noisyImage, fileName.toStdString());

		statusBar()->showMessage("Setting up your image...");
		m_data->noisyImage->normalise();
		m_data->noisyImage->toColourSpace(Denoise::Image::OPP);

		m_data->basicImage = new Denoise::Image(*m_data->noisyImage);
		m_data->resultImage = new Denoise::Image(*m_data->noisyImage);

		statusBar()->showMessage("Image Loaded Successfully...");

		m_viewport->addImage(m_data->noisyImage, "Input");
		m_viewport->displayImage(1);
	}
}

void Sakarnen::saveImage()
{

}


Sakarnen::~Sakarnen()
{

}
