#ifndef DATE_H
#define DATE_H
#include <iostream>

namespace lyz {



class WeekSecond {
public:
    WeekSecond(int week, double sec):
        week(week), second(sec){}
    WeekSecond(){}
    int week;
    double second;
};



class Date {
public:
    Date(int y, int m, int d, double h);
    Date(int y, int m, int d, int h,
         int minute);
    Date(int y, int m, int d, int h,
         int minute, double second);
    Date(){}
    Date(const Date&);

    bool operator < (const Date& rhs);
    bool operator > (const Date& rhs);
    bool operator == (const Date& rhs);
    bool operator != (const Date& rhs){ return !((*this)==rhs); }

    double operator -(const Date& rhs)const{
        return date_sec_diff(*this, rhs);
    }
    friend std::ostream& operator << (
            std::ostream& ost, const Date& date){
        ost << "[" << date.year << "." << date.month << "."
            << date.day << "  " << date.hour << ":"
            << date.minute << ":" << int(date.second)<<"]";
        return ost;
    }

    double diff(const Date& date)const;
    double toJD();
    double toMJD();


    static double date2JD(const Date& date);
    static double date2MJD(const Date& date);
    static double date_sec_diff(const Date& date1, const Date& date2);

    static WeekSecond JD2GPST(double jd);
    static WeekSecond JD2BDST(double jd);
    static WeekSecond date2GPST(const Date& date);
    static WeekSecond date2BDST(const Date& date);

    static WeekSecond GPST2BDST(const WeekSecond& gpst);
    static WeekSecond BDST2GPST(const WeekSecond& gpst);

    static double GPST2JD(const WeekSecond& gpst);
    static double BDST2JD(const WeekSecond& bdst);


    int year,month,day,hour,minute;
    double second;
    double JD;

};



}
#endif // DATE_H
