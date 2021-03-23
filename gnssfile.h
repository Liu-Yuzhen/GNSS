#ifndef GNSSFILE_H
#define GNSSFILE_H
#include "datatype.h"
#include <string>

namespace lyz{
class GNSSFile
{
public:
    GNSSFile(const std::string& path);
    GNSSFile(const char* path);
    GNSSFile();
    virtual ~GNSSFile(){}

    virtual int open(const std::string& path) = 0;
    virtual int isopen(){ return !_map.empty();}

    virtual PositionPtr compute(
            std::string prn, const Date& date) = 0;

    virtual void writeSP3(const std::string& path) = 0;

    std::string path(){ return _path; }

protected:
    std::string _path;
    EhpemerisMap _map;
    std::string _version;
    char _satellie_system;
    std::string _line;
};



class BRDCFile: public GNSSFile
{
public:
    BRDCFile(const std::string& path):GNSSFile(path){ open(path);}
    BRDCFile(const char* path):GNSSFile(path){ open(path); }
    BRDCFile(){}
    virtual ~BRDCFile(){}
    virtual int open(const std::string& path);

    virtual PositionPtr compute(
            std::string prn, const Date& date);

    virtual void writeSP3(const std::string& path);
};





}
#endif // GNSSFILE_H
