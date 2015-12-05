#pragma once

#include "Denoiser\BM3DSettings.h"

#include <QtWidgets\qdockwidget.h>
#include <qgridlayout.h>
#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qframe.h>
#include <qlabel.h>

class SettingsWidgetRight : public QDockWidget
{
	Q_OBJECT
public:
	SettingsWidgetRight(QWidget* parent = nullptr);
	~SettingsWidgetRight();

private slots:
	void previewChanged(bool enabled) { if (enabled){ m_bm3dSettings.previewQuality(); } else { m_bm3dSettings.productionQuality(); } }
	void smoothUniformChanged(bool enabled) { m_bm3dSettings.averageBlocksBasedOnStdWiener = enabled; }
	void adaptivityChanges(bool enabled) { }

private:
	void setupUI();
	void setOptions2Defaults();

	Denoise::BM3DSettings m_bm3dSettings;
	
	QWidget* m_generalWidget;
	QGridLayout* m_generalLayout;
	
	QTabWidget* m_settingsTabs;
	QWidget* m_artisticSettingsTab;
	QWidget* m_technichalSettingsTab;

	QGridLayout* m_artisticSettingsLayout;
	QGridLayout* m_technicalSettingsLayout;

	QLabel* m_dummyLabel1;

	//Artistic Controls
	QLabel* m_artisticQuality;
	QCheckBox* m_artisticPreview;

	//Smoothness
	QLabel* m_artisticSmoothnessLabel;
	QFrame* m_artisticSmoothnessSeparator;
	QLabel* m_artisticSmoothnessLabelAllChannels;
	QSlider* m_artisticSmoothnessSlider;
	QLabel* m_artisticSmoothnessLabelChannel1;
	QDoubleSpinBox* m_artisticSmoothnessChannel1;
	QLabel* m_artisticSmoothnessLabelChannel2;
	QDoubleSpinBox* m_artisticSmoothnessChannel2;
	QLabel* m_artisticSmoothnessLabelChannel3;
	QDoubleSpinBox* m_artisticSmoothnessChannel3;

	//Fine Control
	QLabel* m_artisticFineControlLabel;
	QFrame* m_artisticFineControlSeparator;
	QCheckBox* m_artisticSmoothUniformRegionsOptimally;
	QDoubleSpinBox* m_artisticSmoothUniformRegionsOptimallyStrength;

	//Adaptivity
	QLabel* m_artisticLuminanceAdaptivityLabel;
	QFrame* m_artisticLuminanceAdaptivitySeparator;
	QCheckBox* m_artisticLuminanceAdaptivity;
	QDoubleSpinBox* m_artisticLuminanceAdaptivityStrength;
	QDoubleSpinBox* m_artisticLuminanceAdaptivityScale;
	QLabel* m_artisticLuminanceAdaptivityStrengthLabel;
	QLabel* m_artisticLuminanceAdaptivityScaleLabel;

	//Technical Controls
	QDoubleSpinBox* m_technicalMaxNumThreads;
};

