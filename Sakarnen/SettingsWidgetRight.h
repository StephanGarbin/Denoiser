#pragma once

#include "Denoiser\BM3DSettings.h"

#include <QtWidgets\qdockwidget.h>
#include <qgridlayout.h>
#include <qtabwidget.h>
#include <qcheckbox.h>

class SettingsWidgetRight : public QDockWidget
{
	Q_OBJECT
public:
	SettingsWidgetRight(QWidget* parent = nullptr);
	~SettingsWidgetRight();



private:
	Denoise::BM3DSettings m_bm3dSettings;
	
	QWidget* m_generalWidget;
	QGridLayout* m_generalLayout;
	
	QTabWidget* m_settingsTabs;
	QWidget* m_artisticSettingsTab;
	QWidget* m_technichalSettingsTab;

	QGridLayout* m_artisticSettingsLayout;
	QGridLayout* m_technicalSettingsLayout;

	//Artistic Controls
	QCheckBox* m_artisticPreview;
	QCheckBox* m_artisticSmoothUniformRegionsOptimally;
	QCheckBox* m_artisticLuminanceAdaptivity;

	//Technical Controls

};

