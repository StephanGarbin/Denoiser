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
#include <qslider.h>
#include <qspinbox.h>
#include <qevent.h>

#include "ViewPortScrollArea.h"

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

	bool& displayBasic() { return m_displayBasic; }

protected:
	virtual void wheelEvent(QWheelEvent * e);
	virtual void keyPressEvent(QKeyEvent * e);

private slots:
	void zoomIn();
	void zoomOut();
	void normalSize();

	void resetContrast() { m_imageContrast = 1.0f; setSliderValuesBrightnessContrast(); setTextEditValuesBrightnessContrast(); }
	void resetBrigthness() { m_imageBrightness = 1.0f; setSliderValuesBrightnessContrast(); setTextEditValuesBrightnessContrast(); }

	void setSliderValuesBrightnessContrast();
	void getSliderValuesBrightnessContrast(int t);

	void setTextEditValuesBrightnessContrast();
	void getTextEditValuesBrightnessContrast(double v);

private:


	QGridLayout* m_layoutBrightnessContrast;
	QGridLayout* m_layoutViewerButtons;
	QGridLayout* m_layout;
	ViewPortScrollArea* m_scrollArea;
	QLabel* m_label_imageDisplay;
	QLabel* m_label_input;

	QPushButton* m_buttonZoomIn;
	QPushButton* m_buttonZoomOut;
	QPushButton* m_buttonNormalSize;

	QPushButton* m_buttonResetBrightness;
	QPushButton* m_buttonResetContrast;
	QSlider* m_sliderBrightness;
	QSlider* m_sliderContrast;
	QDoubleSpinBox* m_textEditBrightness;
	QDoubleSpinBox* m_textEditContrast;

	QAction *zoomInAct;
	QAction *zoomOutAct;
	QAction *normalSizeAct;
	
	double m_scaleFactor;
	double m_maxScale;
	double m_minScale;

	float m_imageContrast;
	float m_imageBrightness;

	float sliderScale;

	bool m_displayBasic;

	void scaleImage(double factor);
	void adjustScrollBar(QScrollBar *scrollBar, double factor);

	std::vector<std::pair<std::string, const Denoise::Image*> > m_images;
	size_t m_currentImage;
};

