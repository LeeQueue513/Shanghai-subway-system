#ifndef MYQGRAPHICSVIEW_H
#define MYQGRAPHICSVIEW_H

#include <QGraphicsView>

class MyQGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MyQGraphicsView(QWidget *parent = nullptr);

    void zoomIn();
    void zoomOut();
    void resetZoom();
    void refresh();

private:
    void zoom(double);

signals:

};

#endif // MYQGRAPHICSVIEW_H
