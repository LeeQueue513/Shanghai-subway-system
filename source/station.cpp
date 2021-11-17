#include "station.h"

#include <cmath>

#include <QDebug>

double minLongi;
double minLati;
double maxLongi;
double maxLati;

Station::Station()
{

}

State Station::addEdge(const Station&sta,const QString line_name)
{
    //添加一条从this到sta的位于line_name线上的边
    if(this->edges.count(sta.name)) {  //同一站点仅添加一次
        int len = this->edges[sta.name].line_list.size();
        this->edges[sta.name].line_list.insert(line_name);
        return (len==this->edges[sta.name].line_list.size())?State::ERROR:State::OK;
    }

    Edge edge;
    edge.line_list.insert(line_name);
    if(sta.edges.count(this->name))
        edge.dist = sta.edges[this->name].dist;
    else
        edge.dist = getDistance(*this,sta);

    this->edges[sta.name] = edge;

    qDebug()<<edge.dist;

    return State::OK;
}

void Station::latiLongi2coord()
{
    //    this->pos.setX((longi-minLongi)/(maxLongi-minLongi)*2000);
    //    this->pos.setY((maxLati-lati)/(maxLongi-minLongi)*2000);
    this->coord.setX((this->longi-minLongi)/(maxLongi-minLongi)*3000+30);
    this->coord.setY((maxLati-this->lati)/(maxLati-minLati)*2000+30);
}

QString Station::getBelongLinesText()
{
    QString ret;
    for(auto &i:this->lines)
        ret+=i+' ';
    return ret;
}

static double HaverSin(double theta)
{
    double v = sin(theta / 2);
    return v * v;
}

int getDistance(const Station&s1,const Station&s2)
{
    const double EARTH_RADIUS = 6378.137;

    double rlati1 = s1.lati * M_PI / 180;
    double rlati2 = s2.lati * M_PI / 180;
    double rlongi1 = s1.longi * M_PI / 180;
    double rlongi2 = s2.longi * M_PI / 180;

    double vlongi = abs(rlongi1-rlongi2);
    double vlati = abs(rlati1-rlati2);

    double h = HaverSin(vlati)+cos(rlati1)*cos(rlati2)*HaverSin(vlongi);
    double ret = 2*EARTH_RADIUS*asin(sqrt(h));

    return (int)(ret*1000);
    //    return ret;
}

void updateBound(const double lati,const double longi)
{
    if(minLati>lati)
        minLati=lati;
    else if(maxLati<lati)
        maxLati=lati;
    if(minLongi>longi)
        minLongi=longi;
    else if(maxLongi<longi)
        maxLongi=longi;
}
