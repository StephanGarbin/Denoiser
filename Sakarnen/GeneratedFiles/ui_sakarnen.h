/********************************************************************************
** Form generated from reading UI file 'sakarnen.ui'
**
** Created by: Qt User Interface Compiler version 5.3.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SAKARNEN_H
#define UI_SAKARNEN_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SakarnenClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *SakarnenClass)
    {
        if (SakarnenClass->objectName().isEmpty())
            SakarnenClass->setObjectName(QStringLiteral("SakarnenClass"));
        SakarnenClass->resize(600, 400);
        menuBar = new QMenuBar(SakarnenClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        SakarnenClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(SakarnenClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        SakarnenClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(SakarnenClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        SakarnenClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(SakarnenClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        SakarnenClass->setStatusBar(statusBar);

        retranslateUi(SakarnenClass);

        QMetaObject::connectSlotsByName(SakarnenClass);
    } // setupUi

    void retranslateUi(QMainWindow *SakarnenClass)
    {
        SakarnenClass->setWindowTitle(QApplication::translate("SakarnenClass", "Sakarnen", 0));
    } // retranslateUi

};

namespace Ui {
    class SakarnenClass: public Ui_SakarnenClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAKARNEN_H
