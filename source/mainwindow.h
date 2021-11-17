#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QMap>

#include "station.h"
#include "subwaysystem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initComboBox();
    void drawSubwaySystem();
    void drawTransLine(const QList<QString>&);

    void queryLine();       //查询线路

    void addStation();
    void addLine();
    void addEdge();

private:
    Ui::MainWindow *ui;
    SubwaySystem subsys;
    QGraphicsScene *scene;   //绘图场景

    void drawStation(const Station&,const QColor);
    void drawEdge(const Station&,const Station&,const QColor);
};
#endif // MAINWINDOW_H
