#ifndef MASSFUNCS_H
#define MASSFUNCS_H

#include "filemanager.h"
#include "Eigen/Dense"
using namespace Eigen;


Vector4d* spp(int index, lyz::ObsFile* obs,
              const lyz::Date& date,
              Matrix3d& Qy);


Vector3d xyz2BLH(double x, double y, double z);


Matrix3d ENU(double x, double y, double z);




#endif // MASSFUNCS_H
