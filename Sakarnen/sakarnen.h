#ifndef SAKARNEN_H
#define SAKARNEN_H

#include <QtWidgets/QMainWindow>
#include "ui_sakarnen.h"

class Sakarnen : public QMainWindow
{
	Q_OBJECT

public:
	Sakarnen(QWidget *parent = 0);
	~Sakarnen();

private:
	Ui::SakarnenClass ui;
};

#endif // SAKARNEN_H
