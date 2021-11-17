#include "subwaysystem.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector>

SubwaySystem::SubwaySystem()
{

}

State SubwaySystem::readSubwayFile(QString filepath)
{
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    if(!file.isOpen()){
        //报错
        qDebug()<<filepath<<"打开失败";
        return State::ERROR;
    }
    this->lines.clear();

    minLongi=minLati=2000;
    maxLongi=maxLati=0;

    QTextStream in(&file);
    QString buffer;
    while(true){
        in>>buffer>>buffer; //输入id
        if(in.atEnd())
            break;
        in>>buffer>>buffer; //输入线路名

        Line *line_now = &this->lines[buffer];
        line_now->name = buffer;
        in>>buffer>>buffer; //输入颜色
        line_now->color = QColor(buffer);
        in>>buffer>>buffer; //起始站
        line_now->start_stas.append(buffer);
        in>>buffer; //终点站
        line_now->end_stas.append(buffer);
        in>>buffer>>buffer; //总站数
        line_now->total_stations.append(buffer.toInt());

        QList<QString> sta_list;
        for(int i=0;i<line_now->total_stations.last();i++){
            Station sta;
            in>>buffer>>sta.name>>sta.longi>>sta.lati;
            if(!this->stations.count(sta.name))
                this->stations[sta.name]=sta;
            this->stations[sta.name].lines.insert(line_now->name);
            updateBound(sta.lati,sta.longi);
            sta_list.append(sta.name);
        }
        line_now->sta_list.append(sta_list);
    }
    file.close();

    //统计边的信息
    statisticEdges();

    return State::OK;
}

void SubwaySystem::statisticEdges()
{
    for(auto &line:this->lines.values()){
        for(auto &sta_list:line.sta_list){
            int len = sta_list.size();
            for(int i=0;i<len;i++){
                if(i>0)
                    this->stations[sta_list.at(i)].addEdge(this->stations[sta_list.at(i-1)],line.name);
                if(i<len-1)
                    this->stations[sta_list.at(i)].addEdge(this->stations[sta_list.at(i+1)],line.name);
            }
        }
    }
}

State SubwaySystem::addStation(QString name,double longi,double lati,QSet<QString> line_set)
{
    if(name==""){
        qDebug()<<"尚未输入名称";
        return State::ERROR;
    } else if(this->stations.count(name)){
        qDebug()<<"站点已经存在";
        return State::REPEAT;
    }

    this->stations[name].name = name;
    this->stations[name].lati = lati;
    this->stations[name].longi = longi;
    this->stations[name].lines = line_set;
    this->stations[name].latiLongi2coord();

    return State::OK;
}

State SubwaySystem::addLine(QString name,QColor color)
{
    if(name==""){
        qDebug()<<"尚未输入线路名称";
        return State::ERROR;
    } else if(this->lines.count(name)){
        qDebug()<<"线路已经存在";
        return State::REPEAT;
    }
    this->lines[name].name = name;
    this->lines[name].color = color;

    return State::OK;
}

State SubwaySystem::addEdge(QString sta1,QString sta2,QString line)
{
    this->stations[sta1].lines.insert(line);
    this->stations[sta2].lines.insert(line);
    this->stations[sta1].addEdge(this->stations[sta2],line);
    this->stations[sta2].addEdge(this->stations[sta1],line);
    this->lines[line].total_stations.push_back(2);
    this->lines[line].start_stas.push_back(sta1);
    this->lines[line].end_stas.push_back(sta2);

    QList<QString> sta_list;
    sta_list.push_back(sta1);
    sta_list.push_back(sta2);
    this->lines[line].sta_list.push_back(sta_list);

    return State::OK;
}

QList<QString> SubwaySystem::shortTimePath(const QString start_sta,const QString end_sta)
{
    qDebug()<<"求最短时间";
    //迪杰斯特拉算法求解最短时间路径
    const int INFINITY = INT32_MAX;
    QMap<QString,int> short_time_map;     //距离表
    QMap<QString,bool> final;
    QMap<QString,QList<QString>> ret_line;

    for(auto &i:this->stations) {
        short_time_map[i.name] = INFINITY;
        ret_line[i.name].append(start_sta);
    }
    short_time_map[start_sta] = 0;
    final[start_sta] = true;
    for(auto &i:this->stations[start_sta].edges.keys())
        short_time_map[i] = this->stations[start_sta].edges[i].dist;

    for(int i=1;i<this->stations.size();i++){
        int min_dist = INFINITY;
        QString v;
        for(auto &w:this->stations)
            if(!final[w.name]&&short_time_map[w.name]<min_dist) {
                min_dist = short_time_map[w.name];
                v = w.name;
            }
        if(v==end_sta){
            ret_line[v].append(v);
            break;
        }
        final[v] = true;
        for(auto &w:this->stations)
            if(!final[w.name]&&this->stations[v].edges.count(w.name)&&
                    (min_dist+this->stations[v].edges[w.name].dist<short_time_map[w.name])){
                short_time_map[w.name] = min_dist+this->stations[v].edges[w.name].dist;
                ret_line[w.name] = ret_line[v];
                ret_line[w.name].append(v);
            }
    }

    return ret_line[end_sta];
}

QList<QString> SubwaySystem::lessTransPath(const QString start_sta,const QString end_sta)
{    
    qDebug()<<"求最少换乘";
    //广度优先遍历求解最少换乘路径
    struct Path{
        int dist;
        QString line_name;
        int last_line_id;
        QString trans_sta;
    };
    QList<Path> ret_line;
    QVector<Path> open_list;      //使用QList代替队列,避免元素释放
    for(auto &i:this->stations[start_sta].lines){
        Path path;
        path.dist = 0;
        path.line_name = i;
        path.last_line_id = -1;
        path.trans_sta=start_sta;
        open_list.push_back(path);
    }

    int min_dist = INT32_MAX;
    int index = 0;
    while(index<open_list.size()){
        Path now = open_list.at(index);
        index++;
        if(now.dist>min_dist)
            break;
        for(auto &sta_list:this->lines[now.line_name].sta_list){
            if(sta_list.indexOf(end_sta)>=0){
                min_dist=now.dist;
                ret_line.append(now);
                break;
            }
        }
        if(min_dist==now.dist)
            continue;
        //当前线路上没有终点,则插入中转线路
        for(auto &sta_list:this->lines[now.line_name].sta_list){
            for(auto &sta:sta_list){
                for(auto &each_line:this->stations[sta].lines){
                    if(each_line!=now.line_name){
                        Path path;
                        path.dist = now.dist+1;
                        path.line_name = each_line;
                        path.last_line_id = index-1;
                        path.trans_sta = sta;
                        open_list.push_back(path);
                    }
                }
            }
        }
    }

    QList<QList<QString>> ret_sta_list;
    QList<QList<QString>> ret;
    for(auto &i:ret_line){
        QList<QString> line;
        line.push_back(end_sta);
        line.push_front(i.trans_sta);
        int last_line_id = i.last_line_id;
        while(last_line_id!=-1){
            line.push_front(open_list.at(last_line_id).trans_sta);
            last_line_id = open_list.at(last_line_id).last_line_id;
        }
        ret_sta_list.append(line);
    }
    for(auto &sta_list:ret_sta_list){
        ret.push_back(*new QList<QString>);
        ret.last().push_back(sta_list.first());
        for(int i=0;i<sta_list.size()-1;i++)
            for(auto &sta:getSameLineABPath(sta_list.at(i),sta_list.at(i+1)))
                if(ret.last().last()!=sta)
                    ret.last().push_back(sta);
    }

    min_dist = INT32_MAX;
    index = -1;
    for(int i=0;i<ret.size();i++)
        if(ret.at(i).size()<min_dist){
            min_dist = ret.at(i).size();
            index = i;
        }
    qDebug()<<ret.at(index);
    return ret.at(index);
}

QList<QString> SubwaySystem::getSameLineABPath(const QString&s1,const QString&s2)const
{
    //s1和s2位于同一条线上,找出其中间经过的站点
    QList<QList<QString>> ret_list;
    QList<QString> same_line_list;
    for(auto &i:this->stations[s1].lines){
        for(auto &j:this->stations[s2].lines)
            if(i==j){
                same_line_list.push_back(i);
                break;
            }
    }
    for(auto &same_line:same_line_list)
        for(auto &sta_list:this->lines[same_line].sta_list){
            int index1 = sta_list.indexOf(s1);
            int index2 = sta_list.indexOf(s2);
            if(index1>=0&&index2>=0){
                int delta = (index1>index2)?-1:1;
                ret_list.push_back(*new QList<QString>);
                for(int i=index1;i!=index2;i+=delta)
                    ret_list.last().push_back(sta_list.at(i));          //终点未加上
                ret_list.last().push_back(s2);
                continue;
            }
        }
    if(ret_list.size()){
        int index = 0;
        int min = INT32_MAX;
        for(int i=0;i<ret_list.length();i++)
            if(ret_list.at(i).length()<min){
                min = ret_list.at(i).length();
                index = i;
            }
        return ret_list.at(index);
    }

    //至此,两个站点在同一线路的不同方向
    for(auto &same_line:same_line_list){
        QList<QPair<int,int>> s1_line_index;
        QList<QPair<int,int>> s2_line_index;
        for(int i=0;i<this->lines[same_line].sta_list.size();i++){
            int index1=this->lines[same_line].sta_list.at(i).indexOf(s1);
            if(index1>=0)
                s1_line_index.push_back({i,index1});
            int index2=this->lines[same_line].sta_list.at(i).indexOf(s2);
            if(index2>=0)
                s2_line_index.push_back({i,index2});
        }
        QMap<QPair<int,int>,bool> have_check_list;
        for(int i=0;i<s1_line_index.size();i++){
            const QList<QString>*s1_line=&this->lines[same_line].sta_list.at(s1_line_index.at(i).first);
            int index1=s1_line_index.at(i).second;
            for(int j=0;j<s2_line_index.size();j++){
                if(have_check_list.count({i,j}))
                    continue;
                for(int m=s1_line_index.at(i).second-1;m>=0;m--){
                    QString middle_sta=s1_line->at(m);
                    const QList<QString>*s2_line=&this->lines[same_line].sta_list.at(s2_line_index.at(j).first);
                    int index2=s2_line_index.at(j).second;
                    int middle_index=s2_line->indexOf(middle_sta);
                    if(middle_index>=0){
                        int delta=(index1>m)?-1:1;
                        ret_list.push_back(*new QList<QString>);
                        for(int k=index1;k!=m;k+=delta)
                            ret_list.last().push_back(s1_line->at(k));
                        delta=(middle_index>index2)?-1:1;
                        for(int k=middle_index;k!=index2;k+=delta)
                            ret_list.last().push_back(s2_line->at(k));
                        ret_list.last().push_back(s2_line->at(index2));
                        have_check_list[{i,j}] = true;
                        break;
                    }
                }
                if(have_check_list.count({i,j}))
                    continue;
                for(int m=s1_line_index.at(i).second+1;m<this->lines[same_line].sta_list.at(s1_line_index.at(i).first).size();m++){
                    QString middle_sta=s1_line->at(m);
                    const QList<QString>*s2_line=&this->lines[same_line].sta_list.at(s2_line_index.at(j).first);
                    int index2=s2_line_index.at(j).second;
                    int middle_index=s2_line->indexOf(middle_sta);
                    if(middle_index>=0){
                        int delta=(index1>m)?-1:1;
                        ret_list.push_back(*new QList<QString>);
                        for(int k=index1;k!=m;k+=delta)
                            ret_list.last().push_back(s1_line->at(k));
                        delta=(middle_index>index2)?-1:1;
                        for(int k=middle_index;k!=index2;k+=delta)
                            ret_list.last().push_back(s2_line->at(k));
                        ret_list.last().push_back(s2_line->at(index2));
                        have_check_list[{i,j}] = true;
                        break;
                    }
                }
            }
        }
    }

    int index = 0;
    int min = INT32_MAX;
    for(int i=0;i<ret_list.length();i++)
        if(ret_list.at(i).length()<min){
            min = ret_list.at(i).length();
            index = i;
        }
    return ret_list.at(index);
}
