#include "myqgraphicsview.h"

#include <cmath>
#include <QDebug>
#include <QWheelEvent>

MyQGraphicsView::MyQGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    resetZoom();
}

void MyQGraphicsView::refresh()
{
    this->viewport()->update();
}

void MyQGraphicsView::zoomIn()
{
    zoom(1.2);
}

void MyQGraphicsView::zoomOut()
{
    zoom(0.833);
}

void MyQGraphicsView::zoom(double scaleFactor)
{
    //缩放函数
    qreal factor = transform().scale(scaleFactor,scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.1 || factor > 50)
        return;
    scale(scaleFactor, scaleFactor);
}

void MyQGraphicsView::resetZoom()
{
    qreal init_scale = 1;
    this->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    QMatrix q;
    q.setMatrix(init_scale,this->matrix().m12(),this->matrix().m21(),init_scale,this->matrix().dx(),this->matrix().dy());
    this->setMatrix(q,false);
}
