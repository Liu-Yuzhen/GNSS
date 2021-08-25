#ifndef POSITIONWIDGET_H
#define POSITIONWIDGET_H
#include "gnssfile.h"
#include "Eigen/Dense"
#include <QDialog>
#include <QWidget>
#include "dockwidget.h"
using namespace Eigen;

namespace Ui {
class PositionWidget;
}

class PositionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PositionWidget(
            DockWidget* dock,
            QWidget *parent = nullptr);
    ~PositionWidget();

private slots:
    void openFile();
    void computeClick();
    void clear();

private:
//    Vector4d* compute(const lyz::Date& date,
//                          Matrix3d& Qy);

//    const double w_earth = 7.2921151467e-5;
//    const double c = 299792458;// speed of light
    Ui::PositionWidget *ui;
    lyz::ObsFile* obs;
    DockWidget* _dock;
};

#endif // POSITIONWIDGET_H
