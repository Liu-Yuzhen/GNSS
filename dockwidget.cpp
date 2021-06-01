#include "dockwidget.h"
#include "ui_dockwidget.h"
#include "filemanager.h"
#include <QSize>
#include <QDebug>

DockWidget::DockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DockWidget)
{
    ui->setupUi(this);

    connect(ui->buttonClose, SIGNAL(clicked(bool)),
            this, SLOT(closeFile()));
    connect(ui->buttonCloseAll, SIGNAL(clicked(bool)),
            this, SLOT(closeFileAll()));
}

DockWidget::~DockWidget()
{
    delete ui;
}


void DockWidget::closeFile(){
    int index = selectedIndex();
    if (index < 0)
        return;
    FileManager::instance()->closeFile(index);
    ui->listWidget->setCurrentRow(-1);
    updateElement();
}


void DockWidget::closeFileAll(){
    FileManager::instance()->clearMemory();
    ui->listWidget->setCurrentRow(-1);
    updateElement();
}


/* update file names
 * in the list widget
 */
void DockWidget::updateElement(){
    ui->listWidget->clear();
    ui->listWidget->addItems(FileManager::instance()->getNames());
    this->update();
}


const QListWidget* DockWidget::getListWidget(){
    return ui->listWidget;
}


int DockWidget::selectedIndex(){
    return ui->listWidget->currentRow();

}
