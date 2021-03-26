#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H
#include <QVector>
#include <QDockWidget>
#include <QListWidget>

namespace Ui {
class DockWidget;
}

class DockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit DockWidget(QWidget *parent = nullptr);
    ~DockWidget();

    void updateElement();
    int selectedIndex();
    const QListWidget* getListWidget();

private slots:
    void closeFile();
    void closeFileAll();

private:
    Ui::DockWidget *ui;

    QVector<QString> file_names;
};

#endif // DOCKWIDGET_H
