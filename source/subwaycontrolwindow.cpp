#include "subwaycontrolwindow.h"
#include "ui_subwaycontrolwindow.h"
#include "station.h"

#include <QColorDialog>
#include <QPushButton>
#include <QDebug>
#include <QMessageBox>
#include <QCheckBox>
#include <QGridLayout>

static void (SubwayControlWindow::*const initFun[])() = {
        &SubwayControlWindow::initTab1,
        &SubwayControlWindow::initTab2,
        &SubwayControlWindow::initTab3
        };
static const QString title[] = {
    "添加线路",
    "添加站点",
    "添加连接"
};

SubwayControlWindow::SubwayControlWindow(int index,SubwaySystem* subsys) :
    QWidget(nullptr),
    ui(new Ui::SubwayControlWindow)
{
    ui->setupUi(this);
    setWindowTitle(title[index]);
    setAttribute(Qt::WA_DeleteOnClose);
    this->subsys = subsys;

    ui->stackedWidget->setCurrentIndex(index);
    (this->*(initFun[index]))();
}

SubwayControlWindow::~SubwayControlWindow()
{
    delete ui;
}

void SubwayControlWindow::getColor()
{
    this->color = QColorDialog::getColor(Qt::white, this);
    ui->pushButton->setStyleSheet(QString("background-color: rgb(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue()));
    qDebug()<<color;
}

void SubwayControlWindow::errorNotice(QString prompt)
{
    QMessageBox::critical(this,"错误",prompt);
}

void SubwayControlWindow::rightNotice(QString prompt)
{
    QMessageBox::information(this,"提示",prompt);
}

void SubwayControlWindow::initTab1()
{
    connect(ui->pushButton,&QPushButton::clicked,this,&SubwayControlWindow::getColor);
    connect(ui->pushButton_2,&QPushButton::clicked,this,&SubwayControlWindow::submitTab1);
}
void SubwayControlWindow::initTab2()
{
#if USE_CHECKBOX_LIST
    //添加多选框
    QCheckBox** checkbox_list = new QCheckBox*[this->subsys->lines.size()];
    this->choose_line_list = new bool[this->subsys->lines.size()];
    this->line_list.clear();
    QGridLayout* layout = new QGridLayout(ui->scrollAreaWidgetContents);
    layout->setSpacing(10);
    int i=0;
    for(auto &line:this->subsys->lines){
        this->choose_line_list[i] = false;
        checkbox_list[i] = new QCheckBox(this);
        checkbox_list[i]->setText(line.name);
        this->line_list.push_back(line.name);
        layout->addWidget(checkbox_list[i],i,0);
        connect(checkbox_list[i],&QCheckBox::stateChanged,this,[&](int v){this->choose_line_list[i]=(bool)v;});
        i++;
    }
#else
    ui->widget_4->hide();
#endif
    //设置spinbox范围
    ui->doubleSpinBox->setRange(minLongi,maxLongi);
    ui->doubleSpinBox_2->setRange(minLati,maxLati);
    ui->label_8->setText(QString("经度 [%1,%2]").arg(minLongi).arg(maxLongi));
    ui->label_10->setText(QString("纬度 [%1,%2]").arg(minLati).arg(maxLati));
    connect(ui->pushButton_5,&QPushButton::clicked,this,&SubwayControlWindow::submitTab2);
}
void SubwayControlWindow::initTab3()
{
    //初始化combobox
    QStringList sta_name_list;
    QStringList line_name_list;
    for(auto &i:this->subsys->stations.keys())
        sta_name_list.push_back(i);
    for(auto &i:this->subsys->lines.keys())
        line_name_list.push_back(i);
    std::sort(sta_name_list.begin(),sta_name_list.end(),[](const QString &s1, const QString &s2){
        return (s1.localeAwareCompare(s2) < 0);
    });
    std::sort(line_name_list.begin(),line_name_list.end(),[](const QString &s1, const QString &s2){
        return (s1.length()!=s2.length())?(s1.length()<s2.length()):(s1<s2);
    });
    ui->comboBox->addItems(sta_name_list);
    ui->comboBox_2->addItems(sta_name_list);
    ui->comboBox_3->addItems(line_name_list);

    QLineEdit *line1 = new QLineEdit;
    line1->setPlaceholderText("请选择站点1");
    ui->comboBox->setLineEdit(line1);
    ui->comboBox->lineEdit()->clear();
    QLineEdit *line2 = new QLineEdit;
    line2->setPlaceholderText("请选择站点2");
    ui->comboBox_2->setLineEdit(line2);
    ui->comboBox_2->lineEdit()->clear();

    connect(ui->pushButton_6,&QPushButton::clicked,this,&SubwayControlWindow::submitTab3);
}
void SubwayControlWindow::submitTab1()
{
    this->name = ui->lineEdit->text();
    qDebug()<<this->name;
    qDebug()<<this->color;
    if(this->name==""){
        errorNotice("未输入线路名称！");
        return;
    } else if(this->name.length()>10){
        errorNotice("线路名称不允许超过10个字符\n请重新输入");
        return;
    } else if(this->subsys->lines.count(this->name)){
        errorNotice("线路已存在\n请重新输入");
        return;
    } else if(this->color==QColor::Invalid){
        errorNotice("未选择线路颜色！");
        return;
    }

    this->subsys->addLine(this->name,this->color);
    rightNotice("线路"+this->name+"添加成功！");
    close();    //关闭窗口
}
void SubwayControlWindow::submitTab2()
{
    this->name = ui->lineEdit_3->text();
    if(this->name==""){
        errorNotice("未输入站点名称！");
        return;
    } else if(this->name.length()>10){
        errorNotice("站点名称不允许超过10个字符\n请重新输入");
        return;
    } else if(this->subsys->stations.count(this->name)){
        errorNotice("站点已存在\n请重新输入");
        return;
    }

    this->longi = ui->doubleSpinBox->value();
    this->lati = ui->doubleSpinBox_2->value();

    QSet<QString> line_set;
#if USE_CHECKBOX_LIST
    for(int i=0;i<this->subsys->lines.size();i++)
        if(this->choose_line_list[i]==true)
            line_set.insert(this->line_list.at(i));
    if(line_set.size()==0){
        errorNotice("请至少选择一条线路！");
        return;
    }
#endif
    this->subsys->addStation(this->name,this->longi,this->lati,line_set);
    rightNotice("站点"+this->name+"添加成功！");
    close();    //关闭窗口

    emit done();
}
void SubwayControlWindow::submitTab3()
{
    QString text1,text2,text3;
    text1 = ui->comboBox->lineEdit()->text();
    text2 = ui->comboBox_2->lineEdit()->text();
    text3 = ui->comboBox_3->currentText();
    if(text1==""){
        errorNotice("请输入站点1！");
        return;
    } else if(text2==""){
        errorNotice("请输入站点2！");
        return;
    } else if(this->subsys->stations.count(text1)==0){
        errorNotice("站点1不存在\n请重新输入");
        return;
    } else if(this->subsys->stations.count(text2)==0){
        errorNotice("站点2不存在\n请重新输入");
        return;
    }

    qDebug()<<text1<<text2<<text3;
    this->subsys->addEdge(text1,text2,text3);
    rightNotice("站点"+text1+"到站点"+text2+"的连接添加成功！");
    close();    //关闭窗口

    emit done();
}
