#ifndef DPWIDGET_H
#define DPWIDGET_H
#include <QDialog>
#include <QWidget>
#include "gnssfile.h"
#include "Eigen/Dense"
#include "dockwidget.h"
using namespace Eigen;

namespace Ui {
class DPWidget;
}

class DPWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DPWidget(
            DockWidget* dock,
            QWidget *parent = nullptr);
    ~DPWidget();

private slots:
    void openFile1();
    void openFile2();
    void computeClick();
    void clear();


private:
//    Vector4d* spp(const lyz::Date& date,
//                          Matrix3d& Qy);

    Vector3d* phaseSD(
            const std::vector<std::vector<double>>& ref_phases,
            const std::vector<std::vector<double>>& unk_phases,
            const std::vector<std::vector<Vector4d>>& xyzts,
            Vector4d* xyztref, double wavelength);

    Vector3d* phaseDD(
            const std::vector<std::vector<double>>& ref_phases,
            const std::vector<std::vector<double>>& unk_phases,
            const std::vector<std::vector<Vector4d>>& xyzts,
            Vector4d* xyztref, double wavelength);

    Vector3d* pseudoSD(
            const std::vector<std::vector<double>>& ref_ranges,
            const std::vector<std::vector<double>>& unk_ranges,
            const std::vector<std::vector<Vector4d>>& xyzts,
            Vector4d* xyztref);

    Vector3d* pseudoDD(
            const std::vector<std::vector<double>>& ref_ranges,
            const std::vector<std::vector<double>>& unk_ranges,
            const std::vector<std::vector<Vector4d>>& xyzts,
            Vector4d* xyztref);


    Ui::DPWidget *ui;


    const double w_earth = 7.2921151467e-5;
    const double c = 299792458;// speed of light
    DockWidget* _dock;
    lyz::ObsFile* obsr;
    lyz::ObsFile* obsu;
};

#endif // DPWIDGET_H
