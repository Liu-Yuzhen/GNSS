#ifndef CONTENTWIDGET_H
#define CONTENTWIDGET_H
#include "dockwidget.h"
#include "date.h"
#include <QWidget>

namespace Ui {
class ContentWidget;
}

class ContentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContentWidget(
            DockWidget* dock,
            QWidget *parent = nullptr);

    ~ContentWidget();

    void updateElement();

private slots:
    void compute();
    void clearText();
    void updateElement(int);

private:
    DockWidget* _dock;
    Ui::ContentWidget *ui;
};

#endif // CONTENTWIDGET_H
