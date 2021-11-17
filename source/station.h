#ifndef STATION_H
#define STATION_H

#include <QString>
#include <QSet>
#include <QPointF>
#include <QMap>

//30.9099   31.4101   121.015   121.925
extern double minLongi;
extern double minLati;
extern double maxLongi;
extern double maxLati;

enum class State
{
    ERROR = -1,
    OK = 0,
    REPEAT = 1      //重复
};

struct Edge{
    QSet<QString> line_list;
    int dist = 0;
};

class Station
{
protected:
    QString name;
    double longi, lati;         //站点经纬度
    QPointF coord;              //站点在图上的坐标位置
    QMap<QString,Edge> edges;   //边
    QSet<QString> lines;        //所属的线路

public:
    Station();

    void latiLongi2coord();
    State addEdge(const Station&,const QString);

    QString getBelongLinesText();

    friend int getDistance(const Station&,const Station&);

    friend class Line;
    friend class SubwaySystem;
    friend class MainWindow;
};

int getDistance(const Station&,const Station&);
void updateBound(const double,const double);

#endif // STATION_H
