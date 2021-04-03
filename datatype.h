#ifndef DATATYPE_H
#define DATATYPE_H
#include <map>
#include <vector>
#include <string>
#include <QSharedPointer>
#include "mat.h"
#include "date.h"


namespace lyz {
	
/**************************************************/
class Position {
public:
    Position(Pointd pt);
    Position(Pointd pt,
             double clock_erorr,
             const Date& date);
    Position(){}

    bool operator <(const Position& rhs){
        return date < rhs.date;
    }

    bool operator ==(const Position& rhs){
        return date == rhs.date;
    }

    bool operator >(const Position& rhs){
        return date > rhs.date;
    }



    Pointd pt;
    std::string prn;
    double clock_error;
    Date date;
};
typedef QSharedPointer<Position> PositionPtr;



class Ephemeris
{
public:
    virtual PositionPtr compute(const Date&) = 0;
    std::string prn;
    Date toc;
};



typedef QSharedPointer<Ephemeris> EhpemerisPtr;
typedef std::map<std::string, std::vector<EhpemerisPtr>> EhpemerisMap;
typedef EhpemerisMap::const_iterator GPSephs_const_iterator;


class GPSEhpemeris: public Ephemeris
{
public:
    virtual PositionPtr compute(const Date&);


	double a0, a1, a2;
	double IODE;
	double Crc, Crs;
	double Cuc, Cus;
	double Cic, Cis;
	double delta_n;
	double M0;
	double e;
	double sqrt_a;
	double toe;
	double Omg0;
	double I0;
	double w;//perigee
	double I0dot;
	double Omg0dot;
	double GPSweek;
	double TGD;
	double IODC;
	double codeL2;
	double L2flag;
	double accuracy;
	double health;
	double trans_time_of_message;
	double fit_interval;
protected:
    virtual double toe2jd(const WeekSecond& wksec);
};
typedef QSharedPointer<GPSEhpemeris> GPSEhpemerisPtr;



// bdst
class GPSBEhpemeris: public GPSEhpemeris
{
protected:
    virtual double toe2jd(const WeekSecond& wksec);
};
typedef QSharedPointer<GPSBEhpemeris> GPSBEhpemerisPtr;





class SBASEhpemeris: public Ephemeris
{
public:
    // not implemented
    virtual PositionPtr compute(const Date&);
    // line 1
    double clock_bias;
    double frequency_bias;
    double trans_time_of_message;
    // line 2
    double x;
    double vx;
    double ax;
    double health;

    // line 3
    double y;
    double vy;
    double ay;
    double accuracy;

    // line 4
    double z;
    double vz;
    double az;
    double IODN;

};
typedef QSharedPointer<SBASEhpemeris> SBASEhpemerisPtr;
//class


class GLONASSEphemeris: public Ephemeris
{
public:
    virtual PositionPtr compute(const Date&);

    // line 1
    double clock_bias;
    double frequency_bias;
    double message_frame_time;
    // line 2
    double x;
    double vx;
    double ax;
    double health;

    // line 3
    double y;
    double vy;
    double ay;
    double accuracy;

    // line 4
    double z;
    double vz;
    double az;
    double IODN;
};
typedef QSharedPointer<GLONASSEphemeris> GLONASSEhpemerisPtr;
/**************************************************/





}
#endif // !DATALOADER_H
