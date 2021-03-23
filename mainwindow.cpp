#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datatype.h"
#include "gnssfile.h"
#include <QDebug>
#include <QString>
using namespace std;
using namespace lyz;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    test();


}


void MainWindow::test(){
    QString qstr = "      ";
    cout << qstr.toDouble() << endl;



    cout.precision(9);
    const char* path_brdc = "C:\\Users\\23792\\Desktop\\Qt\\gnss\\data\\brdc3100.20n";
    const char* path_brdm = "C:\\Users\\23792\\Desktop\\Qt\\gnss\\data\\brdm3130.20p";
    const char* path_sp3 = "C:\\Users\\23792\\Desktop\\Qt\\gnss\\data\\igs21304.sp3";
    const char* path_sp3_out = "C:\\Users\\23792\\Desktop\\Qt\\gnss\\brdc3100.20n.sp3";


    GNSSFile* gnssfile = new BRDCFile(path_brdm);

}

MainWindow::~MainWindow()
{
    delete ui;
}

