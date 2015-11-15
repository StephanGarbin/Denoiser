#pragma once

#include <QtWidgets\qdockwidget.h>

class SettingsWidgetRight : public QDockWidget
{
	Q_OBJECT
public:
	SettingsWidgetRight(QWidget* parent = nullptr);
	~SettingsWidgetRight();
};

