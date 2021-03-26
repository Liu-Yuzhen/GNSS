#include "contentwidget.h"
#include "filemanager.h"
#include "ui_contentwidget.h"
#include <QSize>
#include <QDebug>
#include <QDate>
#include <QTime>
#include <QMessageBox>

ContentWidget::ContentWidget(DockWidget* dock, QWidget *parent) :
    QWidget(parent), _dock(dock),
    ui(new Ui::ContentWidget)
{
    ui->setupUi(this);

    connect(ui->buttonCompute,
            SIGNAL(clicked(bool)),
            this, SLOT(compute()));

    connect(ui->buttonClear,
            SIGNAL(clicked(bool)),
            this, SLOT(clearText()));

}


ContentWidget::~ContentWidget()
{
    delete ui;
}


void ContentWidget::clearText(){
    ui->textBrowser_2->setText("");
}


void ContentWidget::updateElement(){
    updateElement(_dock->selectedIndex());
}


void ContentWidget::updateElement(int cur){
    if (cur < 0 ){
        ui->textBrowser->setText("");
        ui->comboBox->clear();
        return;
    }


    ui->textBrowser->setText(
                FileManager::instance()->getContent(cur));

    ui->comboBox->clear();
    ui->comboBox->addItems(
                FileManager::instance()->getPrn(cur));

    lyz::Date date_start =
            FileManager::instance()->getFileDate(cur);

    QDate date(date_start.year, date_start.month,
               date_start.day);
    ui->dateTimeEdit->setDate(date);
    this->update();
}


QString pos2qstring(lyz::PositionPtr pos){
    lyz::Date date = pos->date;
    QString time = QString::number(date.year);
    time += "/" + QString::number(date.month);
    time += "/" + QString::number(date.day);
    time += " " + QString::number(date.hour);
    time += ":";
    if (date.minute < 10)
        time += "0";
    time += QString::number(date.minute);
    QString s = "[" + QString::fromStdString(pos->prn)
            + " "  + time + "]\n";

    s += "x = ";
    s += QString::number(pos->pt._x, 'f', 5);
    s += " km \ny = ";
    s += QString::number(pos->pt._y, 'f', 5);
    s += " km \nz = ";
    s += QString::number(pos->pt._z, 'f', 5);
    s += " km \nclock error = ";
    s += QString::number(pos->clock_error*1e6, 'f', 5);
    s += "μs\n\n";
    return s;
}


void ContentWidget::compute(){
    QString prn = ui->comboBox->currentText();
    if (prn == "")
        return;

    if (prn[0] == 'S' || prn[0] == 'R'){
        QMessageBox::information(
                    this, "Error",
                    "Can't compute GLOANASS",
                    QMessageBox::Yes);
        ui->textBrowser_2->setText("");
        return;
    }

    FileManager* instance = FileManager::instance();
    QDate qdate =  ui->dateTimeEdit->date();

    size_t fileIndex = _dock->selectedIndex();
    lyz::Date selectedDate =
            instance->getFileDate(fileIndex);

    int day = qdate.day();
    int month = qdate.month();
    int y = qdate.year();
    QTime time = ui->dateTimeEdit->time();
    int hour = time.hour();
    int min = time.minute();
    int sec = time.second();
    lyz::Date date(y, month, day, hour, min, sec);

    double dif = date - selectedDate;
    if (dif > 86400 || dif < 0){
        QMessageBox::information(
                    this, tr("Error"),
                    tr("Date time is invalid"),
                    QMessageBox::Yes);
        return;
    }

    lyz::PositionPtr pos =
            instance->compute(fileIndex, prn.toStdString(), date);



    ui->textBrowser_2->setText(ui->textBrowser_2->toPlainText() +
                               pos2qstring(pos));
    update();
}


