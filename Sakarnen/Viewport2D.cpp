#include "Viewport2D.h"

#include "qpixmap.h"
#include "qimage"
#include "qrgb.h"
#include "qscrollbar.h"
#include "qmessagebox.h"

Viewport2D::Viewport2D(QWidget* parent) : QWidget(parent)
{
	//Label for Image Display
	m_label_imageDisplay = new QLabel(this);
	m_label_imageDisplay->setBackgroundRole(QPalette::Base);
	m_label_imageDisplay->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	m_label_imageDisplay->setScaledContents(true);

	//Scroll area to contain image display label
	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setBackgroundRole(QPalette::Dark);
	m_scrollArea->setWidget(m_label_imageDisplay);

	m_label_input = new QLabel(this);
	m_label_input->setText("Noisy Image");

	m_buttonZoomIn = new QPushButton("Zoom In", this);

	m_buttonZoomOut = new QPushButton("Zoom Out", this);

	m_buttonNormalSize = new QPushButton("Fit", this);
		
	m_layout = new QGridLayout(this);

	m_layoutViewerButtons = new QGridLayout();
	m_layoutViewerButtons->addWidget(m_buttonZoomIn, 0, 0);
	m_layoutViewerButtons->addWidget(m_buttonZoomOut, 0, 1);
	m_layoutViewerButtons->addWidget(m_buttonNormalSize, 0, 2);


	m_layout->addLayout(m_layoutViewerButtons, 0, 0);
	m_layout->addWidget(m_label_input, 1, 0);
	m_layout->addWidget(m_scrollArea, 2, 0);

	setup();

	//Set up the widget
	this->setLayout(m_layout);
	this->setWindowTitle("Viewport");
}


Viewport2D::~Viewport2D()
{
}


void Viewport2D::addImage(const Denoise::Image* image, const std::string& name)
{
	m_images.push_back(std::pair<std::string, const Denoise::Image*>(name, image));
}

void Viewport2D::updateImage(size_t idx, const Denoise::Image* image, const std::string& name)
{
	//only update name if necessary
	if (name != "")
	{
		m_images[idx].first = name;
	}

	m_images[idx].second = image;
}

void Viewport2D::displayImage(size_t idx)
{
	//No image loaded, we have to do nothing
	if (idx == 0)
	{
		return;
	}
	
	//Create a QImage
	QImage image(m_images[idx].second->height(),
		m_images[idx].second->width(),
		QImage::Format::Format_ARGB32);

	//Copy Data
	for (size_t row = 0; row < m_images[idx].second->height(); ++row)
	{
		for (size_t col = 0; col < m_images[idx].second->width(); ++col)
		{
			QColor c;
			c.setRgbF(m_images[idx].second->getPixel(0, row, col),
				m_images[idx].second->getPixel(1, row, col),
				m_images[idx].second->getPixel(2, row, col));

			image.setPixel(col, row, c.rgb());
		}
	}

	//Assign pixmap to label
	m_label_imageDisplay->setPixmap(QPixmap::fromImage(image));
	m_label_imageDisplay->adjustSize();
}

void Viewport2D::setup()
{
	m_images.push_back(std::pair<std::string, const Denoise::Image*>("Node Image Loaded", nullptr));
	m_currentImage = 0;

	zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
	zoomInAct->setShortcut(tr("Ctrl++"));
	zoomInAct->setEnabled(false);
	connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

	zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
	zoomOutAct->setShortcut(tr("Ctrl+-"));
	zoomOutAct->setEnabled(false);
	connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

	normalSizeAct = new QAction(tr("&Normal Size"), this);
	normalSizeAct->setShortcut(tr("Ctrl+S"));
	normalSizeAct->setEnabled(false);
	connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));


	connect(m_buttonZoomIn, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(m_buttonZoomOut, SIGNAL(clicked()), this, SLOT(zoomOut()));
	connect(m_buttonNormalSize, SIGNAL(clicked()), this, SLOT(normalSize()));

	m_scaleFactor = 1.0;
}

void Viewport2D::zoomIn()
{
	scaleImage(1.25);
}

void Viewport2D::zoomOut()
{
	scaleImage(0.8);
}

void Viewport2D::normalSize()
{
	m_label_imageDisplay->adjustSize();
	m_scaleFactor = 1.0;
}

void Viewport2D::scaleImage(double factor)
{
	m_scaleFactor *= factor;
	m_label_imageDisplay->resize(m_scaleFactor * m_label_imageDisplay->pixmap()->size());

	adjustScrollBar(m_scrollArea->horizontalScrollBar(), factor);
	adjustScrollBar(m_scrollArea->verticalScrollBar(), factor);

	zoomInAct->setEnabled(m_scaleFactor < 3.0);
	zoomOutAct->setEnabled(m_scaleFactor > 0.333);
}

void Viewport2D::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
	scrollBar->setValue(int(factor * scrollBar->value()
		+ ((factor - 1) * scrollBar->pageStep() / 2)));
}