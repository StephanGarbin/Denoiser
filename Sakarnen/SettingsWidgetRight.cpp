#include "SettingsWidgetRight.h"


SettingsWidgetRight::SettingsWidgetRight(QWidget* parent) : QDockWidget(parent)
{
	const int NUMBER_OF_COLUMNS_IN_GRID = 2;

	m_generalWidget = new QWidget(this);

	m_generalLayout = new QGridLayout(m_generalWidget);

	m_settingsTabs = new QTabWidget(m_generalWidget);
	
	m_artisticSettingsTab = new QWidget();
	m_technichalSettingsTab = new QWidget();

	m_artisticSettingsLayout = new QGridLayout(m_generalWidget);
	m_technicalSettingsLayout = new QGridLayout(m_generalWidget);

	m_artisticQuality = new QLabel("<b>Quality<\b>", m_generalWidget);
	m_artisticQuality->setMaximumHeight(m_artisticQuality->fontMetrics().height());
	m_artisticPreview = new QCheckBox("Preview", m_generalWidget);
	m_artisticSettingsLayout->addWidget(m_artisticQuality);
	m_artisticSettingsLayout->addWidget(m_artisticPreview);

	m_artisticSmoothnessSeparator = new QFrame(m_generalWidget);
	m_artisticSmoothnessSeparator->setFrameShape(QFrame::HLine);
	m_artisticSettingsLayout->addWidget(m_artisticSmoothnessSeparator, 2, 0, 1, NUMBER_OF_COLUMNS_IN_GRID);

	m_artisticSmoothnessLabel = new QLabel("<b>Smoothness<\b>", m_generalWidget);
	m_artisticSmoothnessLabel->setMaximumHeight(m_artisticSmoothnessLabel->fontMetrics().height());
	m_artisticSettingsLayout->addWidget(m_artisticSmoothnessLabel);
	m_dummyLabel1 = new QLabel(" ", m_generalWidget);
	m_dummyLabel1->setMaximumHeight(m_dummyLabel1->fontMetrics().height());
	m_artisticSettingsLayout->addWidget(m_dummyLabel1);

	m_artisticSmoothnessLabelAllChannels = new QLabel("All Channels:", m_generalWidget);
	m_artisticSmoothnessSlider = new QSlider(Qt::Orientation::Horizontal, m_generalWidget);
	m_artisticSmoothnessLabelChannel1 = new QLabel("Luminance", m_generalWidget);
	m_artisticSmoothnessChannel1 = new QDoubleSpinBox(m_generalWidget);
	m_artisticSmoothnessLabelChannel2 = new QLabel("Chrominance 1", m_generalWidget);
	m_artisticSmoothnessChannel2 = new QDoubleSpinBox(m_generalWidget);
	m_artisticSmoothnessLabelChannel3 = new QLabel("Chrominance 2", m_generalWidget);
	m_artisticSmoothnessChannel3 = new QDoubleSpinBox(m_generalWidget);

	m_artisticSettingsLayout->addWidget(m_artisticSmoothnessLabelAllChannels);
	m_artisticSettingsLayout->addWidget(m_artisticSmoothnessSlider);
	m_artisticSettingsLayout->addWidget(m_artisticSmoothnessLabelChannel1);
	m_artisticSettingsLayout->addWidget(m_artisticSmoothnessChannel1);
	m_artisticSettingsLayout->addWidget(m_artisticSmoothnessLabelChannel2);
	m_artisticSettingsLayout->addWidget(m_artisticSmoothnessChannel2);
	m_artisticSettingsLayout->addWidget(m_artisticSmoothnessLabelChannel3);
	m_artisticSettingsLayout->addWidget(m_artisticSmoothnessChannel3);

	m_artisticFineControlSeparator = new QFrame(m_generalWidget);
	m_artisticFineControlSeparator->setFrameShape(QFrame::HLine);
	m_artisticSettingsLayout->addWidget(m_artisticFineControlSeparator, 8, 0, 1, NUMBER_OF_COLUMNS_IN_GRID);
	m_artisticFineControlLabel = new QLabel("<b>Fine Control<\b>", m_generalWidget);
	m_artisticFineControlLabel->setMaximumHeight(m_artisticFineControlLabel->fontMetrics().height());
	
	m_artisticSettingsLayout->addWidget(m_artisticFineControlLabel);
	m_artisticSettingsLayout->addWidget(m_dummyLabel1);

	m_artisticSmoothUniformRegionsOptimally = new QCheckBox("Smooth Uniform Regions", m_generalWidget);
	m_artisticSettingsLayout->addWidget(m_artisticSmoothUniformRegionsOptimally);
	
	m_artisticSmoothUniformRegionsOptimallyStrength = new QDoubleSpinBox(m_generalWidget);
	m_artisticSettingsLayout->addWidget(m_artisticSmoothUniformRegionsOptimallyStrength);

	m_artisticLuminanceAdaptivitySeparator = new QFrame(m_generalWidget);
	m_artisticLuminanceAdaptivitySeparator->setFrameShape(QFrame::HLine);
	m_artisticSettingsLayout->addWidget(m_artisticLuminanceAdaptivitySeparator, 11, 0, 1, NUMBER_OF_COLUMNS_IN_GRID);
	m_artisticLuminanceAdaptivityLabel = new QLabel("<b>Adaptivity<\b>", m_generalWidget);
	m_artisticLuminanceAdaptivityLabel->setMaximumHeight(m_artisticLuminanceAdaptivityLabel->fontMetrics().height());
	m_artisticSettingsLayout->addWidget(m_artisticLuminanceAdaptivityLabel);
	m_artisticSettingsLayout->addWidget(m_dummyLabel1);
	
	m_artisticLuminanceAdaptivity = new QCheckBox("Luminance Adaptivity", m_generalWidget);
	m_artisticSettingsLayout->addWidget(m_artisticLuminanceAdaptivity);
	m_artisticSettingsLayout->addWidget(m_dummyLabel1);

	m_artisticLuminanceAdaptivityStrength = new QDoubleSpinBox(m_generalWidget);
	m_artisticLuminanceAdaptivityScale = new QDoubleSpinBox(m_generalWidget);

	m_artisticLuminanceAdaptivityStrengthLabel = new QLabel("Strength:", m_generalWidget);
	m_artisticLuminanceAdaptivityStrengthLabel->setMaximumHeight(m_artisticLuminanceAdaptivityStrengthLabel->fontMetrics().height());
	m_artisticLuminanceAdaptivityScaleLabel = new QLabel("Scale:", m_generalWidget);
	m_artisticLuminanceAdaptivityScaleLabel->setMaximumHeight(m_artisticLuminanceAdaptivityScaleLabel->fontMetrics().height());

	m_artisticSettingsLayout->addWidget(m_artisticLuminanceAdaptivityStrengthLabel);
	m_artisticSettingsLayout->addWidget(m_artisticLuminanceAdaptivityStrength);

	m_artisticSettingsLayout->addWidget(m_artisticLuminanceAdaptivityScaleLabel);
	m_artisticSettingsLayout->addWidget(m_artisticLuminanceAdaptivityScale);

	m_artisticSettingsTab->setLayout(m_artisticSettingsLayout);
	m_technichalSettingsTab->setLayout(m_technicalSettingsLayout);

	m_settingsTabs->addTab(m_artisticSettingsTab, tr("Artistic"));
	m_settingsTabs->addTab(m_technichalSettingsTab, tr("Technical"));

	m_generalLayout->addWidget(m_settingsTabs);
	m_generalWidget->setLayout(m_generalLayout);

	this->setWidget(m_generalWidget);

	setupUI();
}

void SettingsWidgetRight::setupUI()
{

}

void SettingsWidgetRight::setOptions2Defaults()
{

}

SettingsWidgetRight::~SettingsWidgetRight()
{
}
