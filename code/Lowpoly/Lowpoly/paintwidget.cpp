#include "paintwidget.h"

PaintWidget::PaintWidget(QWidget *parent) :
    QLabel(parent)
{
    init();
    QLabel::QLabel( parent );
}
void PaintWidget::init()
{
    isLeftMousePressed = false;
    isPaintButtonClicked = false;
    brushSize = 30;
    curPosition.setX( -50 );
    curPosition.setY( -50 );
}
void PaintWidget::paintEvent(QPaintEvent *parent)
{
    if( isPaintButtonClicked && isLeftMousePressed ) {
        QPainter painter(paintMask);
        QPen pen(Qt::black,brushSize,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
        painter.setPen(pen);
        painter.drawPoint( curPosition );

        for(int y=0;y<srcImg->height();y++) for(int x=0;x<srcImg->width();x++) {
            if( qRed(paintMask->pixel(QPoint(x,y))) < 10 ) {
                int red_ = qRed(inputImg->pixel(QPoint(x,y)));
                int green_ = qGreen(inputImg->pixel(QPoint(x,y)));
                int blue_ = qBlue(inputImg->pixel(QPoint(x,y)));
				srcImg->setPixel( QPoint(x,y), qRgb(red_/2,green_/2,blue_/2+128) );
                //srcImg->setPixel( QPoint(x,y), qRgb(red_/2+128,green_/2,blue_/2) );
            }
        } // End of For
    }

    this->setPixmap(QPixmap::fromImage(*srcImg));

    QLabel::paintEvent(parent);
}
void PaintWidget::mousePressEvent(QMouseEvent *event)
{
    isLeftMousePressed = true;
    //prePosition = event->pos();

    QLabel::mousePressEvent( event );
}

void PaintWidget::mouseReleaseEvent(QMouseEvent *event)
{
    isLeftMousePressed = false;

    QLabel::mouseReleaseEvent( event );
}

void PaintWidget::mouseMoveEvent(QMouseEvent *event)
{
    if( isLeftMousePressed ) {
        curPosition = event->pos();
    }


    QLabel::mouseMoveEvent(event);
}


void PaintWidget::keyReleaseEvent(QKeyEvent *event)
{
    if( event->key() == Qt::Key_1 )
    {
        // emit keyPlusPressed(); //发送+键被按下的消息
        brushSize++;
    }
    if( event->key() == Qt::Key_2 )
    {
        // emit keyMinusPressed();	//发送-键被按下的消息
        brushSize--;
    }

    QLabel::keyReleaseEvent( event );
}
