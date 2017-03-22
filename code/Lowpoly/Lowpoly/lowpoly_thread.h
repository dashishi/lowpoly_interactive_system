#ifndef LOWPOLYTHREAD_H
#define LOWPOLYTHREAD_H
#include <QThread>
#include "qx_basic.h"
#include <qwidget.h>
#include "EdgeDrawing.h"
#include "Timer.h"
#include "delaunay.h"
using namespace std;

//struct l_Point
//{
//    double x,y;
//}l_Point;
class l_Point
{
public:
    int x,y;
};

class LowpolyThread : public QThread
{
    Q_OBJECT
public:
    explicit LowpolyThread(QObject *parent = 0);
    void run();

	unsigned char*** image_in;
    Mat img_mat;
	double** m_weights;
	vector<int> *points_x;
	vector<int> *points_y;
	vector<int> *ranks;
	int height, width;
	int nr_points;

signals:

	void message(QString,int);

public slots:

};
#endif
