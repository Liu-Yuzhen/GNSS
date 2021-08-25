#include "massfuncs.h"

const double w_earth = 7.2921151467e-5;
const double c = 299792458;// speed of light
const double hd = 41621.712;//40136 + 148.72 * (283.15 - 273.16);
const double hw = 11000;
const double hs = 10;
const double Kd = 155.2 * 1e-7 * (1015.75 / 283.15) * (hd - hs);
const double Kw = 155.2 * 1e-7 * (4810 / pow(283.15, 2)) * (hw - hs) * 11.66;


inline double rad2deg(double rad)
{
    return rad / 3.1415926535 * 180;
}

inline double deg2rad(double deg)
{
    return deg / 180.0 * 3.1415926535;
}

Vector4d* spp(int index, lyz::ObsFile* obs,
              const lyz::Date& date,
              Matrix3d& Qy)
{
    std::vector<double> ranges;
    std::vector<std::string> prns_;


    obs->getSeudoRanges(date, ranges, prns_);


    std::vector<double> pseudoranges;
    std::vector<double> pseudoranges_corrected;
    std::vector<std::string> prns;


    for (size_t i = 0; i < prns_.size(); i++){
        if (prns_[i][0] == 'R' || prns_[i][0] == 'S')
            continue;

        lyz::PositionPtr pos = FileManager::
                instance()->compute(index, prns_[i], date);

        if (pos){
            if (pos->clock_error>1e-3)
                continue;
            pseudoranges.push_back(ranges[i]);
            prns.push_back(prns_[i]);
        }
    }

    if (pseudoranges.size() < 4){
        return nullptr;
    }



    std::vector<Vector4d> xyzt;
    // compute xyzt at the time of emission
    for (size_t i = 0; i < pseudoranges.size(); i++){
        double p = pseudoranges[i];
        lyz::Date ts_date = date;
        double dt = 0.0;

        lyz::PositionPtr pos;

        // for 10 iterations at maximun
        for (size_t k = 0; k < 10; k++){
            ts_date.second = date.second - p / c - dt;
            pos = FileManager::instance()
                    ->compute(index, prns[i], ts_date);
            if (abs(dt - pos->clock_error) < 1e-10){
                break;
            }
            dt = pos->clock_error;
        }

        // earth rotation correction
        lyz::Matd rot = lyz::Matd::creat(p / c * w_earth,
                                         lyz::Axis::zAxis);
        lyz::Pointd pt = 1000 * (rot * pos->pt);

        xyzt.push_back(Vector4d(pt._x, pt._y,
                                pt._z, pos->clock_error));
    }



    Matrix<double, Dynamic, Dynamic> B;
    Matrix<double, Dynamic, Dynamic> L;
    Matrix<double, Dynamic, Dynamic> x;
    Matrix<double, Dynamic, Dynamic> v;
    Matrix<double, Dynamic, Dynamic> P;
    size_t m = pseudoranges.size();
    pseudoranges_corrected = pseudoranges;
    B.resize(m, 4);
    L.resize(m, 1);
    x.setZero(4, 1);
    v.resize(4, 1);
    P.setZero(m, m);

    for (size_t i = 0; i < m; i++){
        P(i, i) = 1;
    }


    Matrix4d Qx;
    Matrix3d rot;
    // for weight
    for (size_t k = 0; k < 10; k++)
    {

        const int max_iteration = 20;

        // bundle adjust
        for (int it = 0; it < max_iteration; it++)
        {
            for (size_t i = 0; i < m; i++)
            {
                double r0 = sqrt((x(0,0) - xyzt[i][0])*(x(0,0) - xyzt[i][0])+
                        (x(1,0) - xyzt[i][1])*(x(1,0) - xyzt[i][1])+
                        (x(2,0) - xyzt[i][2])*(x(2,0) - xyzt[i][2]));

                B(i, 0) = (x(0, 0) - xyzt[i][0]) / r0;
                B(i, 1) = (x(1, 0) - xyzt[i][1]) / r0;
                B(i, 2) = (x(2, 0) - xyzt[i][2]) / r0;
                B(i, 3) = c;

                L(i, 0) = - r0 + pseudoranges_corrected[i] + c * xyzt[i][3] - c * x(3, 0);
            }

            Qx = (B.transpose() * P * B).inverse();
            v = Qx * (B.transpose() * P * L);
            x = x + v;


            if (abs(v(0,0))<1e-12 &&
                    abs(v(1,0))<1e-12 &&
                    abs(v(2,0))<1e-12 &&
                    abs(v(3,0))<1e-10)
            {

                break;
            }
        }


        // update weight matrix
        rot = ENU(x(0, 0), x(1, 0), x(2, 0));
        Vector3d posStation(x(0, 0), x(1, 0), x(2, 0));
        for (size_t kk = 0; kk < m; kk++)
        {
            Vector3d posSat(xyzt[kk][0], xyzt[kk][1], xyzt[kk][2]);
            Vector3d posSat_rot = rot * (posSat - posStation);
            double sinE = posSat_rot[2] /
                    sqrt(posSat_rot[0]*posSat_rot[0] +
                    posSat_rot[1]*posSat_rot[1] +
                    posSat_rot[2] * posSat_rot[2]);

            double E = asin(sinE);
            double Edeg = rad2deg(E);
            double E2 = Edeg * Edeg;
            double jd_dE = deg2rad(sqrt(E2 + 6.25));
            double jd_wE = deg2rad(sqrt(E2 + 2.25));
            pseudoranges_corrected[kk] = pseudoranges[kk] -
                    (Kd / sin(jd_dE) + Kw / sin(jd_wE));

            P(kk, kk) = sinE > 0.5 ? 1: 2*sinE;
        }
    }



    Matrix3d Qx3;
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            Qx3(i,j) = Qx(i,j);
        }
    }

    Qy = rot * Qx3 * rot.transpose();
    return new Vector4d(x(0,0), x(1,0), x(2,0), x(3,0));
}


Vector3d xyz2BLH(double x, double y, double z){
    const double a = 6378245;
    const double e2 = 0.006693421622966;

    double L = atan2(y, x);
    double sqrtx2_y2 = sqrt(x*x + y*y);
    double B = atan2(z, sqrtx2_y2);
    double N;
    for (size_t i = 0; i < 10; i++){
        N = a / sqrt(1 - e2 * pow(sin(B), 2));
        double B1 = atan2(z + N * e2 * sin(B), sqrtx2_y2);

        if (abs(B1 - B) < 1e-8)
            break;
        B = B1;
    }

    double H = sqrtx2_y2 / cos(B) - N;
    return Vector3d(B, L, H);
}


Matrix3d ENU(double x, double y, double z){
    const double pi = 3.1415926535897932384626433832795028841971694;

    Vector3d blh = xyz2BLH(x, y, z);
    AngleAxisd axisz(-pi/2 - blh[1], Vector3d(0, 0, 1));
    Matrix3d rotz = axisz.matrix();

    AngleAxisd axisx(-pi/2 + blh[0], Vector3d(1, 0, 0));
    Matrix3d rotx = axisx.matrix();

    return rotx * rotz;
}

