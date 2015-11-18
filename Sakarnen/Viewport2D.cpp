#include "Viewport2D.h"

#include "qpixmap.h"
#include "qimage"
#include "qrgb.h"
#include "qscrollbar.h"
#include "qmessagebox.h"
#include <qstring.h>

#include "common.h"

Viewport2D::Viewport2D(QWidget* parent) : QWidget(parent)
{
	sliderScale = 1000.0f;
	m_maxScale = 10.0f;
	m_minScale = 0.1f;

	m_imageBrightness = 1.0f;
	m_imageContrast = 1.0f;

	//Label for Image Display
	m_label_imageDisplay = new QLabel(this);
	m_label_imageDisplay->setBackgroundRole(QPalette::Base);
	m_label_imageDisplay->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	m_label_imageDisplay->setScaledContents(true);

	//Scroll area to contain image display label
	m_scrollArea = new ViewPortScrollArea(this);
	m_scrollArea->setBackgroundRole(QPalette::Dark);
	m_scrollArea->setWidget(m_label_imageDisplay);


	//Label to display input source
	m_label_input = new QLabel(this);
	m_label_input->setText("Noisy Image");

	//buttons for zooming
	m_buttonZoomIn = new QPushButton("Zoom In", this);
	m_buttonZoomOut = new QPushButton("Zoom Out", this);
	m_buttonNormalSize = new QPushButton("Fit", this);
		
	m_buttonResetBrightness = new QPushButton("Brightness", this);
	m_buttonResetContrast = new QPushButton("Contrast", this);
	m_textEditBrightness = new QDoubleSpinBox(this);
	m_textEditContrast = new QDoubleSpinBox(this);
	m_textEditBrightness->setMinimum(-1.0);
	m_textEditBrightness->setMaximum(1.0);
	m_textEditContrast->setMaximum(4.0);
	m_textEditContrast->setMinimum(0.0);

	m_textEditBrightness->setSingleStep(0.25);
	m_textEditContrast->setSingleStep(0.25);

	m_sliderBrightness = new QSlider(Qt::Horizontal, this);
	m_sliderContrast = new QSlider(Qt::Horizontal, this);

	m_sliderBrightness->setMinimum(-1.0 * sliderScale);
	m_sliderBrightness->setMaximum(1 * sliderScale);
	

	m_sliderContrast->setMinimum(0.0);
	m_sliderContrast->setMaximum(4.0 * sliderScale);

	setSliderValuesBrightnessContrast();
	setTextEditValuesBrightnessContrast();

	//layouts
	m_layout = new QGridLayout(this);

	m_layoutViewerButtons = new QGridLayout();
	m_layoutViewerButtons->addWidget(m_buttonZoomIn, 0, 0);
	m_layoutViewerButtons->addWidget(m_buttonZoomOut, 0, 1);
	m_layoutViewerButtons->addWidget(m_buttonNormalSize, 0, 2);

	m_layoutBrightnessContrast = new QGridLayout(this);
	m_layoutBrightnessContrast->addWidget(m_buttonResetBrightness, 0, 0);
	m_layoutBrightnessContrast->addWidget(m_textEditBrightness, 0, 1);
	m_layoutBrightnessContrast->addWidget(m_sliderBrightness, 0, 2);
	m_layoutBrightnessContrast->addWidget(m_buttonResetContrast, 0, 3);
	m_layoutBrightnessContrast->addWidget(m_textEditContrast, 0, 4);
	m_layoutBrightnessContrast->addWidget(m_sliderContrast, 0, 5);

	m_layout->addLayout(m_layoutBrightnessContrast, 0, 0);
	m_layout->addLayout(m_layoutViewerButtons, 1, 0);
	m_layout->addWidget(m_label_input, 2, 0);
	m_layout->addWidget(m_scrollArea, 3, 0);

	setup();

	//Set up the widget
	this->setLayout(m_layout);
	this->setWindowTitle("Viewport");

	m_displayBasic = true;
}


Viewport2D::~Viewport2D()
{
}

void Viewport2D::setSliderValuesBrightnessContrast()
{
	m_sliderBrightness->setValue(m_imageBrightness * sliderScale);
	m_sliderContrast->setValue(m_imageContrast * sliderScale);
}

void Viewport2D::getSliderValuesBrightnessContrast(int t)
{
	m_imageBrightness = (float)m_sliderBrightness->value() / sliderScale;
	m_imageContrast = (float)m_sliderContrast->value() / sliderScale;

	setTextEditValuesBrightnessContrast();
	displayImage(m_currentImage);
}

void Viewport2D::setTextEditValuesBrightnessContrast()
{
	m_textEditBrightness->setValue(m_imageBrightness);
	m_textEditContrast->setValue(m_imageContrast);
}

void Viewport2D::getTextEditValuesBrightnessContrast(double v)
{
	m_imageBrightness = m_textEditBrightness->value();
	m_imageContrast = m_textEditContrast->value();

	setSliderValuesBrightnessContrast();
	displayImage(m_currentImage);
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

	QPointF temp = m_scrollArea->getMoveAmount();
	
	//Create a QImage
	QImage image(m_images[idx].second->width(),
		m_images[idx].second->height(),
		QImage::Format::Format_ARGB32);

	//Copy Data
	for (size_t row = 0; row < m_images[idx].second->height(); ++row)
	{
		for (size_t col = 0; col < m_images[idx].second->width(); ++col)
		{
			QColor c;

			c.setRgbF(clamp<>(m_imageContrast * m_images[idx].second->getPixel(0, row, col) + m_imageBrightness - 1.0f, 0.0f, 1.0f),
				clamp<>(m_imageContrast * m_images[idx].second->getPixel(1, row, col) + m_imageBrightness - 1.0f, 0.0f, 1.0f),
				clamp<>(m_imageContrast * m_images[idx].second->getPixel(2, row, col) + m_imageBrightness - 1.0f, 0.0f, 1.0f));

			image.setPixel(col, row, c.rgb());
		}
	}

	//Assign pixmap to label
	m_label_imageDisplay->setPixmap(QPixmap::fromImage(image));
	m_label_imageDisplay->adjustSize();
	m_label_input->setText(QString::fromUtf8(m_images[idx].first.c_str()));

	m_currentImage = idx;
	scaleImage(1.0f);

	m_scrollArea->setMoveAmount(temp);
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

	//Brightness & Contrast Controls
	connect(m_buttonResetBrightness, SIGNAL(clicked()), this, SLOT(resetBrigthness()));
	connect(m_buttonResetContrast, SIGNAL(clicked()), this, SLOT(resetContrast()));

	connect(m_textEditBrightness, SIGNAL(valueChanged(double)), this, SLOT(getTextEditValuesBrightnessContrast(double)));
	connect(m_textEditContrast, SIGNAL(valueChanged(double)), this, SLOT(getTextEditValuesBrightnessContrast(double)));

	connect(m_sliderBrightness, SIGNAL(valueChanged(int)), this, SLOT(getSliderValuesBrightnessContrast(int)));
	connect(m_sliderContrast, SIGNAL(valueChanged(int)), this, SLOT(getSliderValuesBrightnessContrast(int)));

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

void Viewport2D::wheelEvent(QWheelEvent * e)
{
	if (e->delta() < 0.0)
	{
		if (m_scaleFactor > m_minScale)
		{
			scaleImage(1.0 / std::abs((e->delta()) / 100.0));
		}
	}
	else
	{
		if (m_scaleFactor < m_maxScale)
		{
			scaleImage((std::abs(e->delta()) / 100.0));
		}
	}
}

void Viewport2D::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_1)
	{
		if (m_images.size() > 1)
		{
			displayImage(1);
		}
	}
	else if (e->key() == Qt::Key_2)
	{
		if (m_images.size() > 1)
		{
			if (m_displayBasic)
			{
				displayImage(2);
			}
			else
			{
				displayImage(3);
			}
		}
	}
}
