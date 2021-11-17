#ifndef SUBWAYSYSTEM_H
#define SUBWAYSYSTEM_H

#include <QVector>
#include <QSet>
#include <QMap>

#include "../5_2_subway/station.h"
#include "../5_2_subway/line.h"

class SubwaySystem
{
protected:
    QMap<QString,Station> stations;     //所有站点的集合
    QMap<QString,Line> lines;           //所有线路集合

public:
    SubwaySystem();

    State readSubwayFile(QString);
    void statisticEdges();

    State addStation(QString,double longi,double lati,QSet<QString>);
    State addLine(QString,QColor);
    State addEdge(QString,QString,QString);

    QList<QString> shortTimePath(const QString,const QString);
    QList<QString> lessTransPath(const QString,const QString);

    QList<QString> getSameLineABPath(const QString&,const QString&)const;

    friend class MainWindow;
    friend class SubwayControlWindow;
};

#endif // SUBWAYSYSTEM_H
