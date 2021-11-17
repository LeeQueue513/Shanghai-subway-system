#ifndef LINE_H
#define LINE_H

#include <QList>
#include <QSet>
#include <QColor>

#include "station.h"

class Line
{
protected:
    QString name;
    QColor color;
    QList<int> total_stations;                 //总站数
    QList<QString> start_stas,end_stas;        //起点站和终点站
    QList<QList<QString>> sta_list;            //站点集合,此处设置是为了避免一条线有多个分支的情况

public:
    Line(){};

    friend class SubwaySystem;
    friend class MainWindow;
    friend class SubwayControlWindow;
};


#endif // LINE_H
