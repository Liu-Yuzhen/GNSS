#include "positionwidget.h"
#include "ui_positionwidget.h"
#include "filemanager.h"
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <iostream>

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


    Vector4d* xyzt = compute(date);
    if (!xyzt)
        return;


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
    str += "clock error = " + QString::number((*xyzt)[3]*1e6, 'f', 3) + " μs\n\n";
    ui->textBrowser->setText(ui->textBrowser->toPlainText() + str);

    delete xyzt;
}



Vector4d* PositionWidget::compute(const lyz::Date& date){
    int index = _dock->selectedIndex();
    if (index < 0){
        QMessageBox::information(
                    this, "Error",
                    "No navigation file selected!",
                    QMessageBox::Yes);
        return nullptr;
    }



    std::vector<double> ranges;
    std::vector<std::string> prns_;


    obs->getSeudoRanges(date, ranges, prns_);


    std::vector<double> pseudoranges;
    std::vector<std::string> prns;


    for (size_t i = 0; i < prns_.size(); i++){
        if (prns_[i][0] == 'R' || prns_[i][0] == 'S')
            continue;

        lyz::PositionPtr pos = FileManager::
                instance()->compute(index, prns_[i], date);



        if (pos){
            pseudoranges.push_back(ranges[i]);
            prns.push_back(prns_[i]);
        }
    }

    if (pseudoranges.size() < 4){
        QMessageBox::information(
                    this, "Error",
                    "Navigation file has not enough ephemeris!",
                    QMessageBox::Yes);
        return nullptr;
    }



    std::vector<Vector4d> xyzt;
    // compute xyzt at the time of emission
    for (size_t i = 0; i < pseudoranges.size(); i++){
        double p = pseudoranges[i];
        lyz::Date ts_date = date;
        double dt = 0.0;

        lyz::PositionPtr pos;
        // for 10 iterations at maximun
        for (size_t k = 0; k < 10; k++){
            ts_date.second = date.second - p / c - dt;
            pos = FileManager::instance()
                    ->compute(index, prns[i], ts_date);
            if (abs(dt - pos->clock_error) < 1e-10){
                break;
            }
            dt = pos->clock_error;
        }
        // earth rotation correction
        lyz::Matd rot = lyz::Matd::creat(p/c*w_earth,
                                         lyz::Axis::zAxis);
        lyz::Pointd pt = 1000 * (rot * pos->pt);

        xyzt.push_back(Vector4d(pt._x, pt._y,
                                pt._z, pos->clock_error));
    }




    Matrix<double, Dynamic, Dynamic> B;
    Matrix<double, Dynamic, Dynamic> L;
    Matrix<double, Dynamic, Dynamic> x;
    Matrix<double, Dynamic, Dynamic> v;
    size_t m = pseudoranges.size();
    B.resize(m, 4);
    L.resize(m, 1);
    x.resize(4, 1);
    v.resize(4, 1);
    for (size_t i = 0; i < 4; i++){
        x(i, 0) = 0;
    }


    int max_iteration = 20;
    for (int it = 0; it < max_iteration; it++){
        for (size_t i = 0; i < m; i++){
            double r0 = sqrt((x(0,0) - xyzt[i][0])*(x(0,0) - xyzt[i][0])+
                    (x(1,0) - xyzt[i][1])*(x(1,0) - xyzt[i][1])+
                    (x(2,0) - xyzt[i][2])*(x(2,0) - xyzt[i][2]));
            B(i, 0) = (x(0, 0) - xyzt[i][0]) / r0;
            B(i, 1) = (x(1, 0) - xyzt[i][1]) / r0;
            B(i, 2) = (x(2, 0) - xyzt[i][2]) / r0;
            B(i, 3) = c;

            L(i, 0) = - r0 + pseudoranges[i] + c * xyzt[i][3] - c * x(3, 0);
        }

        v = (B.transpose() * B).inverse() * (B.transpose() * L);
        x = x + v;

        if (v.norm() < 1e-4)
            break;
    }



    return new Vector4d(x);
}



void PositionWidget::clear(){
    ui->textBrowser->setText("");
}



PositionWidget::~PositionWidget()
{
    delete obs;
    delete ui;
}
