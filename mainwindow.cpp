#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datatype.h"
#include "filemanager.h"
#include "gnssfile.h"
#include "QFileInfo"
#include <QString>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QSize>
#include <QStringList>
using namespace std;
using namespace lyz;


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



    QToolBar* toolBar = addToolBar("Tool Bar");
    toolBar->addAction(ui->actionOpen);
    toolBar->addAction(ui->actionSave);


    connect(_dock->getListWidget(), SIGNAL(currentRowChanged(int)),
            _content, SLOT(updateElement(int)));

    connect(ui->actionOpen, SIGNAL(triggered(bool)),
            this, SLOT(openFile()));

    connect(ui->actionSave, SIGNAL(triggered(bool)),
            this, SLOT(saveFile()));




    FileManager::instance()->addObserver(new ListObserver(_dock));
    FileManager::instance()->addObserver(new WidgetObserver(_content));
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

