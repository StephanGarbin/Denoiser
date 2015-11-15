#pragma once

#include <utility>
#include <vector>
#include <string>

#include <QtWidgets\qwidget.h>
#include <QtWidgets\qscrollarea.h>
#include <QtWidgets\qlabel.h>
#include <QtWidgets\qgridlayout.h>

#include <qimage.h>
#include <qaction.h>
#include <qpushbutton.h>

#include "Denoiser\Image.h"

class Viewport2D : public QWidget
{
	Q_OBJECT
public:
	Viewport2D(QWidget* parent = nullptr);
	~Viewport2D();

	void addImage(const Denoise::Image* image, const std::string& name);
	void updateImage(size_t idx, const Denoise::Image* image, const std::string& name = "");
	void displayImage(size_t idx);

	void setup();

private slots:
	void zoomIn();
	void zoomOut();
	void normalSize();

private:

	QGridLayout* m_layoutViewerButtons;
	QGridLayout* m_layout;
	QScrollArea* m_scrollArea;
	QLabel* m_label_imageDisplay;
	QLabel* m_label_input;

	QPushButton* m_buttonZoomIn;
	QPushButton* m_buttonZoomOut;
	QPushButton* m_buttonNormalSize;

	QAction *zoomInAct;
	QAction *zoomOutAct;
	QAction *normalSizeAct;
	
	double m_scaleFactor;

	void scaleImage(double factor);
	void adjustScrollBar(QScrollBar *scrollBar, double factor);

	std::vector<std::pair<std::string, const Denoise::Image*> > m_images;
	size_t m_currentImage;
};

