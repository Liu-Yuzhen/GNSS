#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datatype.h"
#include "filemanager.h"
#include "gnssfile.h"
#include "QFileInfo"
#include "dpwidget.h"
#include <QString>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QSize>
#include <QStringList>
#include <iostream>
using namespace std;
using namespace lyz;
using namespace Eigen;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _dock = new DockWidget(this);
    _content = new ContentWidget(_dock, this);


    this->setCentralWidget(_content);
    this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea,
                        _dock, Qt::Orientation::Vertical);


    connect(_dock->getListWidget(), SIGNAL(currentRowChanged(int)),
            _content, SLOT(updateElement(int)));

    connect(ui->actionOpen, SIGNAL(triggered(bool)),
            this, SLOT(openFile()));

    connect(ui->actionSave, SIGNAL(triggered(bool)),
            this, SLOT(saveFile()));

    connect(ui->actionSPP, SIGNAL(triggered(bool)),
            this, SLOT(positionStatic()));

    connect(ui->actionSD_DD, SIGNAL(triggered(bool)),
            this, SLOT(SDDD()));


    FileManager::instance()->addObserver(new ListObserver(_dock));
    FileManager::instance()->addObserver(new WidgetObserver(_content));


}


void MainWindow::positionStatic(){
    PositionWidget* posw = new PositionWidget(_dock);
    posw->show();
}


void MainWindow::SDDD(){
    DPWidget* dpw = new DPWidget(_dock);
    dpw->show();
}


void MainWindow::openFile(){


    //QFileDialog diag(this,tr("Open File"),"", tr("*.*"));

    QStringList pathlist = QFileDialog::getOpenFileNames(
                this, tr("Choose files"),QString(), "*.*");

    if (pathlist.empty())
        return;

    for (int i = 0; i < pathlist.size(); i++){

        QString path = pathlist[i];
        QFileInfo file = path;
        if (file.suffix() == "sp3"){
            GNSSFile* f = new SP3File(path.toStdString());
            if (f->isopen())
                FileManager::instance()->addFile(f);
            else{
                QMessageBox::information(
                            this, "Error",
                            "Unkown format: " + path,
                            QMessageBox::Yes);
                delete f;
            }

        }
        else{
        GNSSFile* f = new BRDCFile(path.toStdString());
        if (f->isopen()){
            FileManager::instance()->addFile(f);
        }
        else{
            QMessageBox::information(
                        this, "Error",
                        "Unkown format: " + path,
                        QMessageBox::Yes);
            delete f;
        }

    }
    }

}


void MainWindow::saveFile(){
    int index = _dock->selectedIndex();
    if (index < 0){
        QMessageBox::information(
                    this,tr("Error"),
                    tr("No file selected"),
                    QMessageBox::Yes);
        return;
    }


    QString path =
            QFileDialog::getSaveFileName(this, tr("Save File"),
                                         "", "*.sp3");
    if (FileManager::instance()->saveFile(index, path) == 0){
        QMessageBox::information(this, "", "Saved to " + path,
                                 QMessageBox::Yes);
    }
    else{
        QMessageBox::information(this, "Error", "Failed",
                                 QMessageBox::Yes);
    }

}



MainWindow::~MainWindow()
{
    delete ui;
}

/*
void MainWindow::test(){
//    vector<double> pseudoranges =
//    {
//    21741790.1704,
//    21320969.2344,
//    23794626.2684,
//    21203769.5634,
//    24890957.9314,
//    22640674.8744,
//    24354384.4394,
//    24728467.2154,
//    24722288.3654
//    };

//    vector<Vector4d> xyzt;
//    xyzt.push_back(Vector4d(-21978.582050,6513.607689,
//                               13504.295578,0.444565));
//    xyzt.push_back(Vector4d(-328.327580,22429.100003,
//                               13924.621027,-25.092010));
//    xyzt.push_back(Vector4d(-19920.875213,16918.484035,
//                            -4775.999316, -82.580366));
//    xyzt.push_back(Vector4d(-7810.865273, 14609.548479,
//                            21406.360716, -538.491847));
//    xyzt.push_back(Vector4d(-20210.643674, 13636.861800,
//                            -10059.196650, 349.516052));
//    xyzt.push_back(Vector4d(-15965.299281, -3267.611640,
//                            20913.166095, -285.392628));
//    xyzt.push_back(Vector4d(13722.136447, 6259.794908,
//                                21831.134878, -229.889945));
//    xyzt.push_back(Vector4d(16566.866450, 16687.918788,
//                              12365.630421, 16.682070));
//    xyzt.push_back(Vector4d(9975.540519, 24611.541121,
//                            1901.221414, 44.719126));


//    size_t m = pseudoranges.size();
//    //size_t n = 4; // x, y, z, tr



//    double c = 299792458;// speed of light
//    Matrix<double, Dynamic, Dynamic> B;
//    Matrix<double, Dynamic, Dynamic> L;
//    Matrix<double, Dynamic, Dynamic> x;
//    Matrix<double, Dynamic, Dynamic> v;
//    B.resize(m, 4);
//    L.resize(m, 1);
//    x.resize(4, 1);
//    v.resize(4, 1);
//    for (size_t i = 0; i < 4; i++){
//        x(i, 0) = 0;
//        v(i, 0) = 1;
//    }


//    while(v.norm() > 1e-3){

//        for (size_t i = 0; i < m; i++){
//            double r0 = sqrt((x(0,0) - xyzt[i][0]*1000)*(x(0,0) - xyzt[i][0]*1000)+
//                    (x(1,0) - xyzt[i][1]*1000)*(x(1,0) - xyzt[i][1]*1000)+
//                    (x(2,0) - xyzt[i][2]*1000)*(x(2,0) - xyzt[i][2]*1000));
//            B(i, 0) = (x(0, 0) - xyzt[i][0]*1000) /r0;
//            B(i, 1) = (x(1, 0) - xyzt[i][1]*1000) / r0;
//            B(i, 2) = (x(2, 0) - xyzt[i][2]*1000) / r0;
//            B(i, 3) = c;

//            L(i, 0) = - r0 + (pseudoranges[i] + c * xyzt[i][3]*1e-6);
//        }

//        v = (B.transpose() * B).inverse() * (B.transpose() * L);
//        x = x + v;
//    }
    const char* path_obs = "C:\\Users\\23792\\Desktop\\Qt\\gnss\\data\\obs\\obs\\leij3100.20o";
    const char* path_brdc = "C:\\Users\\23792\\Desktop\\Qt\\gnss\\data\\brdc3100.20n";


    ObsFile* obs = new ObsFile(path_obs);


    FileManager::instance()->addFile(new BRDCFile(path_brdc));

    Date date(2020, 11, 5, 0, 0, 30);
    vector<double> ranges;
    vector<std::string> prns;



    obs->getSeudoRanges(date, ranges, prns);


    vector<double> pseudoranges;
    vector<Vector4d> xyzt;


    for (size_t i = 0; i < prns.size(); i++){
        if (prns[i][0] == 'R' || prns[i][0] == 'S')
            continue;

        PositionPtr pos = FileManager::instance()->compute(0, prns[i], date);
        if (pos){
            pseudoranges.push_back(ranges[i]);
            xyzt.push_back(Vector4d(pos->pt._x*1e3, pos->pt._y*1e3,
                                    pos->pt._z*1e3, pos->clock_error*1e-6));
        }

    }



    double c = 299792458;// speed of light
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
        v(i, 0) = 1;
    }


    while(v.norm() > 1e-3){

        for (size_t i = 0; i < m; i++){
            double r0 = sqrt((x(0,0) - xyzt[i][0])*(x(0,0) - xyzt[i][0])+
                    (x(1,0) - xyzt[i][1])*(x(1,0) - xyzt[i][1])+
                    (x(2,0) - xyzt[i][2])*(x(2,0) - xyzt[i][2]));
            B(i, 0) = (x(0, 0) - xyzt[i][0]) / r0;
            B(i, 1) = (x(1, 0) - xyzt[i][1]) / r0;
            B(i, 2) = (x(2, 0) - xyzt[i][2]) / r0;
            B(i, 3) = c;

            L(i, 0) = - r0 + (pseudoranges[i] + c * xyzt[i][3]);
        }

        v = (B.transpose() * B).inverse() * (B.transpose() * L);
        x = x + v;
    }

}






*/
