#pragma once

#include <QtWidgets/QScrollArea>

class Viewport2D : public QScrollArea
{
	Q_OBJECT
public:
	Viewport2D();
	~Viewport2D();
};

