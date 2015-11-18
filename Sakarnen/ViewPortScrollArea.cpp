#include "ViewPortScrollArea.h"
#include <qpoint.h>
#include <qmessagebox.h>
#include <qscrollbar.h>

ViewPortScrollArea::ViewPortScrollArea(QWidget* parent) : QScrollArea(parent)
{
	m_ATLPressed = false;
	m_mouseLeftPressed = false;
}


ViewPortScrollArea::~ViewPortScrollArea()
{
}

void ViewPortScrollArea::mousePressEvent(QMouseEvent * e)
{
	if (e->button() == Qt::LeftButton)
	{
		m_mouseLeftPressed = true;
	}

	m_mousePreviousPosition = e->localPos();

}

void ViewPortScrollArea::mouseReleaseEvent(QMouseEvent * e)
{
	if (e->button() == Qt::LeftButton)
	{
		m_mouseLeftPressed = false;
	}

	m_mousePreviousPosition = e->localPos();
}

void ViewPortScrollArea::mouseMoveEvent(QMouseEvent * e)
{
	if (m_mouseLeftPressed && m_ATLPressed)
	{
		QPointF moveAmount = e->localPos() - m_mousePreviousPosition;
		
		this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() + moveAmount.y());
		this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() + moveAmount.x());
		m_mousePreviousPosition = e->localPos();
	}
}

void ViewPortScrollArea::setMoveAmount(QPointF moveAmount)
{
	this->verticalScrollBar()->setValue(moveAmount.y());
	this->horizontalScrollBar()->setValue(moveAmount.x());
}

QPointF ViewPortScrollArea::getMoveAmount()
{
	QPoint temp;
	temp.setY(this->verticalScrollBar()->value());
	temp.setX(this->horizontalScrollBar()->value());

	return temp;
}

void ViewPortScrollArea::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Alt)
	{
		m_ATLPressed = true;
	}
	else
	{
		e->ignore();
	}
}

void ViewPortScrollArea::keyReleaseEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Alt)
	{
		m_ATLPressed = false;
	}
	else
	{
		e->ignore();
	}
}

void ViewPortScrollArea::wheelEvent(QWheelEvent * e)
{
	e->ignore();
}