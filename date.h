#ifndef DATE_H
#define DATE_H

namespace lyz {




class Date {
public:
    Date(int y, int m, int d, double h);
    Date(int y, int m, int d, int h,
         int minute);
    Date(int y, int m, int d, int h,
         int minute, double second);
    Date(){}
    Date(const Date&);

    double diff(const Date& date)const;
    double toJD();
    double toMJD();


    int year,month,day,hour,minute;
    double second;
    double week_second;
    int week_number;
    double JD;

};


class GPST {
public:
    GPST(int week, double sec);
    int week;
    double second;
};

double date2JD(const Date& date);
double date2MJD(const Date& date);
double date_sec_diff(const Date& date1, const Date& date2);

GPST JD2GPST(double jd);
GPST date2GPST(const Date& date);
double GPST2JD(const GPST& gpst);


}
#endif // DATE_H
