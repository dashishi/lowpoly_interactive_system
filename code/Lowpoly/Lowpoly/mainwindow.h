#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include <qmessagebox.h>
#include <qkeyeventtransition.h>
#include <qwidget.h>
#include <qstring.h>
#include <iostream>
#include <qlayout.h>
#include <vector>
#include <cv.h>
#include <math.h>
#include <qfiledialog.h>


#include "paintwidget.h"
#include "qx_basic.h"
#include "get_weights.h"
#include "lowpoly.h"
#include "lowpoly_thread.h"
#include "delaunay.h"
#include "SLIC_corner.h"
#include "Eigen/Sparse"
#include "Eigen/Eigen"
#include "Timer.h"
#include "blend.h"
#include "SparseMatrix.h"

using namespace std;
using namespace cv;
using namespace Eigen;
using namespace myg;
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

	QString path;

	int width;
	int height;

	PaintWidget *imgshow;

	vector<int> points_x;
    vector<int> points_y;
    vector<int> ranks;
private:
	Ui::MainWindowClass ui;

    unsigned char*** image_in;
    double** m_weights;
	QImage *imgInput;
	Mat image_mat;
	double dist;
	int node_size;
	int saturValue;
    QImage *inputBackup;
    QImage *imgTemp;
    QImage *paintMask;

	//vector<VERTEX2D> draw_points;
	void generate(int value , int sValue , double dis);

public slots:
    void open();
    void lowpoly();
    void setTextValue(int value);
    void setSaturValue(int value);
	void setDistValue(int value);
    void paint();
    void clear();
    void save();
    void weights();
    void original_image();
	void render();

protected slots:
    void finish_lowpoly();
};



#endif // MAINWINDOW_H
