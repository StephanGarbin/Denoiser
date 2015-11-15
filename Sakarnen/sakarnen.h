#ifndef SAKARNEN_H
#define SAKARNEN_H

#include <memory>

#include <QtWidgets/QMainWindow>
#include <qaction.h>
#include <qmenu.h>
#include <qstring.h>
#include <qmenubar.h>
#include <qstatusbar.h>

#include "Viewport2D.h"
#include "SettingsWidgetRight.h"

struct SakarnenData;

class Sakarnen : public QMainWindow
{
	Q_OBJECT

public:
	Sakarnen(QWidget *parent = 0);
	~Sakarnen();

private:
	Viewport2D* m_viewport;
	SettingsWidgetRight* m_settingsWidget;
	
	void setupGeneral();

	//Menus
	void setupMenus();

	QMenu* m_fileMenu;

	QAction* m_actionLoadImage;
	QAction* m_actionSaveImage;

	//App Data
	std::shared_ptr<SakarnenData> m_data;

private slots:
	void loadImage();
	void saveImage();
};

#endif // SAKARNEN_H
