#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "contentwidget.h"
#include "dockwidget.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void openFile();
    void saveFile();

private:
    void test();

    DockWidget* _dock;
    ContentWidget* _content;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
