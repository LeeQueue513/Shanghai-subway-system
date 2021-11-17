#ifndef SUBWAYCONTROLWINDOW_H
#define SUBWAYCONTROLWINDOW_H

#include <QWidget>
#include <QString>
#include <QColor>

#include "subwaysystem.h"

#define USE_CHECKBOX_LIST 0

namespace Ui {
class SubwayControlWindow;
}

class SubwayControlWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SubwayControlWindow(int,SubwaySystem*);
    ~SubwayControlWindow();

    void initTab1();
    void initTab2();
    void initTab3();
    void submitTab1();
    void submitTab2();
    void submitTab3();

    void errorNotice(QString);
    void rightNotice(QString);

private:
    Ui::SubwayControlWindow *ui;

    QString name;
    QColor color;
    SubwaySystem*subsys;
    double longi,lati;

#if USE_CHECKBOX_LIST
    bool* choose_line_list;
    QStringList line_list;
#endif

    void getColor();

signals:
    void done();
};

#endif // SUBWAYCONTROLWINDOW_H
