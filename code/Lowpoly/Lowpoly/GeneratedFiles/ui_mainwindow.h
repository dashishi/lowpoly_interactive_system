/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.1.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionLowpoly;
    QAction *actionWeights;
    QAction *actionOrigin;
    QAction *actionRendering;
    QWidget *centralWidget;
    QSlider *slider;
    QLineEdit *lineEdit;
    QPushButton *paint;
    QPushButton *clear;
    QLineEdit *distValue;
    QSlider *distSlider;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName(QStringLiteral("MainWindowClass"));
        MainWindowClass->resize(600, 181);
        actionOpen = new QAction(MainWindowClass);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionSave = new QAction(MainWindowClass);
        actionSave->setObjectName(QStringLiteral("actionSave"));
        actionLowpoly = new QAction(MainWindowClass);
        actionLowpoly->setObjectName(QStringLiteral("actionLowpoly"));
        actionWeights = new QAction(MainWindowClass);
        actionWeights->setObjectName(QStringLiteral("actionWeights"));
        actionOrigin = new QAction(MainWindowClass);
        actionOrigin->setObjectName(QStringLiteral("actionOrigin"));
        actionRendering = new QAction(MainWindowClass);
        actionRendering->setObjectName(QStringLiteral("actionRendering"));
        centralWidget = new QWidget(MainWindowClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        slider = new QSlider(centralWidget);
        slider->setObjectName(QStringLiteral("slider"));
        slider->setGeometry(QRect(10, 30, 160, 19));
        slider->setMinimum(200);
        slider->setMaximum(3000);
        slider->setOrientation(Qt::Horizontal);
        lineEdit = new QLineEdit(centralWidget);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(180, 30, 31, 20));
        paint = new QPushButton(centralWidget);
        paint->setObjectName(QStringLiteral("paint"));
        paint->setGeometry(QRect(230, 30, 75, 23));
        clear = new QPushButton(centralWidget);
        clear->setObjectName(QStringLiteral("clear"));
        clear->setGeometry(QRect(310, 30, 75, 23));
        distValue = new QLineEdit(centralWidget);
        distValue->setObjectName(QStringLiteral("distValue"));
        distValue->setGeometry(QRect(180, 80, 31, 20));
        distSlider = new QSlider(centralWidget);
        distSlider->setObjectName(QStringLiteral("distSlider"));
        distSlider->setGeometry(QRect(10, 80, 160, 19));
        distSlider->setMaximum(2000);
        distSlider->setOrientation(Qt::Horizontal);
        MainWindowClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindowClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 23));
        MainWindowClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindowClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindowClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindowClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindowClass->setStatusBar(statusBar);

        mainToolBar->addAction(actionOpen);
        mainToolBar->addAction(actionSave);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionLowpoly);
        mainToolBar->addAction(actionRendering);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionWeights);
        mainToolBar->addAction(actionOrigin);

        retranslateUi(MainWindowClass);

        QMetaObject::connectSlotsByName(MainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        MainWindowClass->setWindowTitle(QApplication::translate("MainWindowClass", "MainWindow", 0));
        actionOpen->setText(QApplication::translate("MainWindowClass", "Open", 0));
        actionSave->setText(QApplication::translate("MainWindowClass", "Save", 0));
        actionLowpoly->setText(QApplication::translate("MainWindowClass", "Lowpoly", 0));
        actionWeights->setText(QApplication::translate("MainWindowClass", "Weights", 0));
        actionOrigin->setText(QApplication::translate("MainWindowClass", "Origin", 0));
        actionRendering->setText(QApplication::translate("MainWindowClass", "Render", 0));
        paint->setText(QApplication::translate("MainWindowClass", "Paint", 0));
        clear->setText(QApplication::translate("MainWindowClass", "Clear", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
