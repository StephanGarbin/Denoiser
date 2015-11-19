#include "SettingsWidgetRight.h"


SettingsWidgetRight::SettingsWidgetRight(QWidget* parent) : QDockWidget(parent)
{
	m_generalWidget = new QWidget(this);

	m_generalLayout = new QGridLayout(m_generalWidget);

	m_settingsTabs = new QTabWidget(m_generalWidget);
	
	m_artisticSettingsTab = new QWidget();
	m_technichalSettingsTab = new QWidget();

	m_artisticSettingsLayout = new QGridLayout(m_generalWidget);
	m_technicalSettingsLayout = new QGridLayout(m_generalWidget);

	m_artisticPreview = new QCheckBox("Preview", m_generalWidget);
	m_artisticSettingsLayout->addWidget(m_artisticPreview);

	m_artisticSmoothUniformRegionsOptimally = new QCheckBox("Smooth Uniform Regions More", m_generalWidget);
	m_artisticSettingsLayout->addWidget(m_artisticSmoothUniformRegionsOptimally);

	m_artisticLuminanceAdaptivity = new QCheckBox("Luminance Adaptivity", m_generalWidget);
	m_artisticSettingsLayout->addWidget(m_artisticLuminanceAdaptivity);

	m_artisticSettingsTab->setLayout(m_artisticSettingsLayout);
	m_technichalSettingsTab->setLayout(m_technicalSettingsLayout);

	m_settingsTabs->addTab(m_artisticSettingsTab, tr("Artistic"));
	m_settingsTabs->addTab(m_technichalSettingsTab, tr("Technical"));

	m_generalLayout->addWidget(m_settingsTabs);
	m_generalWidget->setLayout(m_generalLayout);

	this->setWidget(m_generalWidget);
}


SettingsWidgetRight::~SettingsWidgetRight()
{
}
