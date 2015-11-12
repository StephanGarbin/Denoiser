#include "sakarnen.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Sakarnen w;
	w.show();
	return a.exec();
}
