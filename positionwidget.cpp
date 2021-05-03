#include "positionwidget.h"
#include "ui_positionwidget.h"
#include "filemanager.h"
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include "massfuncs.h"

PositionWidget::PositionWidget(DockWidget* dock, QWidget *parent) :
    QWidget(parent), _dock(dock),obs(nullptr),
    ui(new Ui::PositionWidget)
{
    ui->setupUi(this);

    // delete when closed
    this->setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->buttonCompute, SIGNAL(clicked(bool)),
            this, SLOT(computeClick()));

    connect(ui->buttonOpen, SIGNAL(clicked(bool)),
            this, SLOT(openFile()));

    connect(ui->buttonClear, SIGNAL(clicked(bool)),
            this, SLOT(clear()));
}


void PositionWidget::openFile(){
    QString path = QFileDialog::getOpenFileName(
                this, tr("Choose files"),QString(), "*.20o");

    if (path == "")
        return;

    if (obs != nullptr)
            delete obs;
    obs = new lyz::ObsFile(path.toStdString());


    if (obs->isopen())
        ui->textBrowser_2->setText(path);
    else
    {
        QMessageBox::information(
                    this, "Error",
                    "Failed to open file: " + path,
                    QMessageBox::Yes);
    }
}



void PositionWidget::computeClick(){
    if (!obs){
        QMessageBox::information(
                    this, "Error",
                    "No observation file selected!",
                    QMessageBox::Yes);
        return;
    }

    QDate qdate =  ui->dateTimeEdit->date();

    int day = qdate.day();
    int month = qdate.month();
    int y = qdate.year();
    QTime time = ui->dateTimeEdit->time();
    int hour = time.hour();
    int min = time.minute();
    int sec = time.second();
    lyz::Date date(y, month, day, hour, min, sec);

    // get the closet
    date = obs->getClosetDate(date);

    QDate dt(date.year, date.month, date.day);
    QTime t(date.hour, date.minute, date.second);
    ui->dateTimeEdit->setDate(dt);
    ui->dateTimeEdit->setTime(t);


    int index = _dock->selectedIndex();
    if (index < 0){
        QMessageBox::information(
                    this, "Error",
                    "No navigation file selected!",
                    QMessageBox::Yes);

        return;
    }

    Matrix3d Q;
    Vector4d* xyzt = spp(index, obs, date, Q);

    if (xyzt == nullptr)
    {
        QMessageBox::information(
                    this, "Error",
                    "Navigation file has not enough ephemeris!",
                    QMessageBox::Yes);
        return;
    }



    QString str = "[";
    str += QString::number(date.year) + "/" +  QString::number(date.month) + "/" +  QString::number(date.day) + " ";
    str +=  QString::number(date.hour) + ":";

    if (date.minute < 10)
        str += "0";
    str += QString::number(date.minute) + ":";

    if (date.second < 10)
        str += "0";
    str += QString::number(date.second) + "]\n";

    str += "x = " + QString::number((*xyzt)[0], 'f', 3) + " m\n";
    str += "y = " + QString::number((*xyzt)[1], 'f', 3) + " m\n";
    str += "z = " + QString::number((*xyzt)[2], 'f', 3) + " m\n";
    str += "clock error = " + QString::number((*xyzt)[3]*1e6, 'f', 3) + " μs\n";
    str += "qxx = " + QString::number(sqrt(Q(0,0)), 'f', 3) + ", qyy = "  + QString::number(sqrt(Q(1,1)), 'f', 3)
            + ", qzz = " + QString::number(sqrt(Q(2,2)), 'f', 3) + "\n";
    str += "PDOP = " + QString::number(sqrt(Q(0,0) + Q(1,1) + Q(2,2)), 'f', 3) + "\n\n";

    ui->textBrowser->setText(ui->textBrowser->toPlainText() + str);

    delete xyzt;
}



void PositionWidget::clear(){
    ui->textBrowser->setText("");
}



PositionWidget::~PositionWidget()
{
    delete obs;
    delete ui;
}
