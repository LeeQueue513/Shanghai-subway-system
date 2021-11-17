#include <QDebug>
#include <QString>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <random>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "subwaysystem.h"
#include "myqgraphicsview.h"
#include "subwaycontrolwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("上海地铁换乘系统");
    srand(0);

    //菜单栏
    connect(ui->actionAddStation,&QAction::triggered,this,&MainWindow::addStation);
    connect(ui->actionAddLine,&QAction::triggered,this,&MainWindow::addLine);
    connect(ui->actionAddEdge,&QAction::triggered,this,&MainWindow::addEdge);
    connect(ui->actionReturnSubway,&QAction::triggered,this,&MainWindow::drawSubwaySystem);
    connect(ui->actionZoomIn,&QAction::triggered,ui->myQGraphicsView,&MyQGraphicsView::zoomIn);
    connect(ui->actionZoomOut,&QAction::triggered,ui->myQGraphicsView,&MyQGraphicsView::zoomOut);

    //工具栏
    QPushButton *zoom_in = new QPushButton(this);
    zoom_in->setIcon(QIcon(":/img/src/zoom_in.svg"));
    zoom_in->setToolTip("放大");
    ui->toolBar->addWidget(zoom_in);
    connect(zoom_in,&QPushButton::clicked,ui->myQGraphicsView,&MyQGraphicsView::zoomIn);
    QPushButton *zoom_out = new QPushButton(this);
    zoom_out->setIcon(QIcon(":/img/src/zoom_out.svg"));
    zoom_out->setToolTip("缩小");
    ui->toolBar->addWidget(zoom_out);
    connect(zoom_out,&QPushButton::clicked,ui->myQGraphicsView,&MyQGraphicsView::zoomOut);
    QPushButton *return_subway = new QPushButton(this);
    return_subway->setIcon(QIcon(":/img/src/subway.svg"));
    return_subway->setToolTip("返回地铁线路图");
    ui->toolBar->addWidget(return_subway);
    connect(return_subway,&QPushButton::clicked,this,&MainWindow::drawSubwaySystem);

    //连接按钮
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::queryLine);

    //调整场景
    this->scene = new QGraphicsScene;
    scene->setBackgroundBrush(Qt::white);
    scene->setSceneRect(0,0,3080,2080);

    //设置画板
    ui->myQGraphicsView->setRenderHint(QPainter::Antialiasing);     //精致绘图
    ui->myQGraphicsView->setScene(scene);
    ui->myQGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    //读入地铁信息
    this->subsys.readSubwayFile(":/text/src/subway_info.txt");
    //求坐标
    for(auto &i:subsys.stations)
        i.latiLongi2coord();

    initComboBox();
    drawSubwaySystem();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initComboBox()
{
    QStringList sta_name_list;
    for(auto &i:this->subsys.stations.keys())
        sta_name_list.append(i);
    //    qDebug()<<this->subsys.stations.keys().size();
    std::sort(sta_name_list.begin(),sta_name_list.end(),[](const QString &s1, const QString &s2){
        return (s1.localeAwareCompare(s2) < 0);
    });
    ui->comboBox->addItems(sta_name_list);
    ui->comboBox_2->addItems(sta_name_list);

    QLineEdit *line1 = new QLineEdit;
    line1->setPlaceholderText("请选择起始站");
    ui->comboBox->setLineEdit(line1);
    ui->comboBox->lineEdit()->clear();
    QLineEdit *line2 = new QLineEdit;
    line2->setPlaceholderText("请选择终点站");
    ui->comboBox_2->setLineEdit(line2);
    ui->comboBox_2->lineEdit()->clear();
}

void MainWindow::drawSubwaySystem()
{
    this->scene->clear();
    ui->myQGraphicsView->refresh();

    bool repeat;
    for(auto &line:this->subsys.lines){
        for(int i=0;i<line.sta_list.size();i++){
            for(int j=0;j<line.sta_list.at(i).size()-1;j++){
                QString s1=line.sta_list.at(i).at(j),s2=line.sta_list.at(i).at(j+1);
                repeat = false;
                //检查是否重复
                for(int m=0;m<i;m++){
                    for(int n=0;n<line.sta_list.at(m).size()-1;n++){
                        if(line.sta_list.at(m).at(n)==s1&&line.sta_list.at(m).at(n+1)==s2){
                            repeat = true;
                            break;
                        }
                    }
                    if(repeat)
                        break;
                }
                if(repeat)
                    continue;
                else
                    drawEdge(this->subsys.stations[s1],this->subsys.stations[s2],line.color);
            }
        }
    }

    for(auto &i:subsys.stations)
        drawStation(i,(i.lines.size()==1)?this->subsys.lines[*i.lines.begin()].color:Qt::black);
}

void MainWindow::drawTransLine(const QList<QString>& trans_line)
{
    //绘图重置
    this->scene->clear();
    ui->myQGraphicsView->refresh();
    //文本框重置
    ui->textBrowser->clear();

    QString line_old = *this->subsys.stations[trans_line.at(0)].edges[trans_line.at(1)].line_list.begin();
    QColor color = this->subsys.lines[line_old].color;
    drawEdge(this->subsys.stations[trans_line.at(0)],this->subsys.stations[trans_line.at(1)],color);
    drawStation(this->subsys.stations[trans_line.at(0)],color);
    ui->textBrowser->setText("共经过"+QString::number(trans_line.size())+"站");
    ui->textBrowser->append("起点: "+trans_line.at(0));
    ui->textBrowser->append("        ↓ 乘坐"+this->subsys.lines[line_old].name);
    for(int i=1;i<trans_line.size()-1;i++){
        ui->textBrowser->append("\t"+trans_line.at(i));
        bool line_name_same=false;
        for(auto &i:this->subsys.stations[trans_line.at(i)].edges[trans_line.at(i+1)].line_list)
            if(i==line_old){
                line_name_same=true;
                break;
            }
        if(line_name_same) {
            //线是一致的,则判断是否为站内换乘即可
            bool line_same=false;
            for(auto &sta_list:this->subsys.lines[line_old].sta_list){
                for(int k=1;k<sta_list.size()-1;k++){
                    if((trans_line.at(i-1)==sta_list.at(k-1)&&trans_line.at(i)==sta_list.at(k)&&trans_line.at(i+1)==sta_list.at(k+1))||
                            (trans_line.at(i+1)==sta_list.at(k-1)&&trans_line.at(i)==sta_list.at(k)&&trans_line.at(i-1)==sta_list.at(k+1))){
                        line_same = true;
                        break;
                    }
                }
                if(line_same)
                    break;
            }
            if(line_same)
                ui->textBrowser->append("        ↓");
            else
                ui->textBrowser->append("        ↓ 站内换乘");
            color = this->subsys.lines[line_old].color;
        }
        else {
            line_old = *this->subsys.stations[trans_line.at(i)].edges[trans_line.at(i+1)].line_list.begin();
            ui->textBrowser->append("        ↓ 换乘"+this->subsys.lines[line_old].name);
            color = Qt::black;
        }
        drawEdge(this->subsys.stations[trans_line.at(i)],this->subsys.stations[trans_line.at(i+1)],this->subsys.lines[line_old].color);
        drawStation(this->subsys.stations[trans_line.at(i)],color);
    }
    drawStation(this->subsys.stations[trans_line.last()],this->subsys.lines[line_old].color);
    ui->textBrowser->append("终点: "+trans_line.last());
    ui->textBrowser->textCursor().setPosition(0);
    ui->textBrowser->setTextCursor(ui->textBrowser->textCursor());
}

void MainWindow::drawEdge(const Station&s1,const Station&s2,const QColor color)
{
    QGraphicsLineItem *line = new QGraphicsLineItem();
    QPointF pos1,pos2;

    if(s1.lines.size()>1){
        pos1.setX(s1.coord.x()+(1.0*rand()/RAND_MAX-0.5)*4+3);
        pos1.setY(s1.coord.y()+(1.0*rand()/RAND_MAX-0.5)*4+3);
    }
    else{
        pos1.setX(s1.coord.x()+3);
        pos1.setY(s1.coord.y()+3);
    }

    if(s2.lines.size()>1){
        pos2.setX(s2.coord.x()+(1.0*rand()/RAND_MAX-0.5)*4+3);
        pos2.setY(s2.coord.y()+(1.0*rand()/RAND_MAX-0.5)*4+3);
    }
    else{
        pos2.setX(s2.coord.x()+3);
        pos2.setY(s2.coord.y()+3);
    }
    line->setLine(QLineF(pos1,pos2));
    line->setPen(QPen(color));
    this->scene->addItem(line);
}

void MainWindow::drawStation(const Station&sta,const QColor color)
{
    QPen elli_pen;
    elli_pen.setColor(color);
    QGraphicsEllipseItem * elli = new QGraphicsEllipseItem();
    elli->setRect(QRect(sta.coord.x(),sta.coord.y(),6,6));
    elli->setPen(elli_pen);
    elli->setBrush(QBrush(Qt::white));
    QString info = "站点名称: "+sta.name+"\n站点经纬度: "+
            QString::number(sta.longi,'f',3)+
            ","+QString::number(sta.lati,'f',3)+
            "\n站点所属线路: ";
    if(sta.lines.size())
        for(auto &i:sta.lines)
            info+=i+' ';
    else
        info+="无";
    info+="\n相邻站点:";
    if(sta.edges.size())
        for(auto &i:sta.edges.keys()){
            info+="\n "+i;
            if(i.length()<=3)
                info+="      ";
            info+="\t相距"+QString::number(sta.edges[i].dist/1000.0)+"km";
        }
    else
        info+="无";
    //    elli->setToolTip(info);
    this->scene->addItem(elli);

    QGraphicsTextItem * text = new QGraphicsTextItem();
    text->setPlainText(sta.name);
    text->setPos(sta.coord.x(),sta.coord.y()+1);
    text->setFont(QFont("consolas",4,1));
    text->setToolTip(info);
    this->scene->addItem(text);
}

void MainWindow::queryLine()
{
    QString start_sta = ui->comboBox->lineEdit()->text();
    QString end_sta = ui->comboBox_2->lineEdit()->text();
    qDebug()<<start_sta<<" "<<end_sta;

    if(start_sta=="") {
        QMessageBox::critical(this,"错误","尚未输入起点站");
        return;
    } else if(end_sta==""){
        QMessageBox::critical(this,"错误","尚未输入终点站");
        return;
    } else if(this->subsys.stations.count(start_sta)==0) {
        QMessageBox::critical(this,"错误","站点\""+start_sta+"\"不存在\n请重新输入");
        return;
    } else if(this->subsys.stations.count(end_sta)==0) {
        QMessageBox::critical(this,"错误","站点\""+end_sta+"\"不存在\n请重新输入");
        return;
    } else if(start_sta==end_sta){
        QMessageBox::critical(this,"错误","起点站和终点站相同\n请重新输入");
        return;
    }

    QList<QString> ret_line;
    if(ui->radioButton->isChecked())
        ret_line = this->subsys.shortTimePath(start_sta,end_sta);
    else
        ret_line = this->subsys.lessTransPath(start_sta,end_sta);
    qDebug()<<ret_line;
    drawTransLine(ret_line);

    //    int line_old;
    //    ui->textBrowser->clear();
    //    ui->textBrowser->setText("起点: "+ret_line.at(0));
    //    line_old = this->subsys.stations[ret_line.at(0)].edges[ret_line.at(1)].line_id_list.at(0);
    //    ui->textBrowser->append("        ↓ 乘坐"+this->subsys.lines[line_old].name);
    //    for(int i=1;i<ret_line.size()-1;i++){
    //        ui->textBrowser->append("\t"+ret_line.at(i));
    //        bool same=false;
    //        for(auto &i:this->subsys.stations[ret_line.at(i)].edges[ret_line.at(i+1)].line_id_list)
    //            if(i==line_old){
    //                same=true;
    //                break;
    //            }
    //        if(!same) {
    //            line_old = this->subsys.stations[ret_line.at(i)].edges[ret_line.at(i+1)].line_id_list.first();
    //            ui->textBrowser->append("        ↓ 换乘"+this->subsys.lines[line_old].name);
    //        }
    //        else
    //            ui->textBrowser->append("        ↓");
    //    }
    //    ui->textBrowser->append("终点: "+ret_line.last());
}

void MainWindow::addLine()
{
    SubwayControlWindow *sub_window = new SubwayControlWindow(0,&this->subsys);
    sub_window->show();
}

void MainWindow::addStation()
{
    SubwayControlWindow *sub_window = new SubwayControlWindow(1,&this->subsys);
    sub_window->show();
    connect(sub_window,&SubwayControlWindow::done,this,&MainWindow::drawSubwaySystem);
}

void MainWindow::addEdge()
{
    SubwayControlWindow *sub_window = new SubwayControlWindow(2,&this->subsys);
    sub_window->show();
    connect(sub_window,&SubwayControlWindow::done,this,&MainWindow::drawSubwaySystem);
}
