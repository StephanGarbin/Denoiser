#pragma once

#include <qscrollarea.h>
#include <qwidget.h>
#include <QtGui\qevent.h>
#include <qpoint.h>

class ViewPortScrollArea : public QScrollArea
{
	Q_OBJECT
public:
	ViewPortScrollArea(QWidget* parent = nullptr);
	~ViewPortScrollArea();

protected:
	virtual void mouseMoveEvent(QMouseEvent * e);

	virtual void mousePressEvent(QMouseEvent * e);
	virtual void mouseReleaseEvent(QMouseEvent * e);

	virtual void keyPressEvent(QKeyEvent * e);
	virtual void keyReleaseEvent(QKeyEvent * e);

	void wheelEvent(QWheelEvent * e);

	bool m_ATLPressed;
	bool m_mouseLeftPressed;
	QPointF m_mousePreviousPosition;
};

