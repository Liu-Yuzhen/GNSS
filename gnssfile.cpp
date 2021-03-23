#include "gnssfile.h"
#include <assert.h>
#include <fstream>
#include <iostream>

namespace lyz{


inline std::string trim(const std::string str) {
    size_t start = str.find_first_not_of(" ");
    size_t end = str.find_last_not_of(" ") + 1;
    return str.substr(start, end - start);
}


// from "1.231234D-02"-like string to float
inline double str2f(const std::string& str) {
    int len = str.length();
    double d = atof(str.substr(0, len - 4).c_str());
    int expo = atof(str.substr(len - 3).c_str());
    return d * pow(10, expo);
}



void read_brdc_gps_like_entry(
        std::ifstream& ifs,EhpemerisMap& m,
        std::string& line){

    GPSEhpemerisPtr ehp(new GPSEhpemeris);

    // pad zero
    if (line[0] == ' ')
        ehp->prn = "0";
    ehp->prn += trim(line.substr(0, 2));

    ehp->toc.year = atoi(line.substr(3, 2).c_str()) + 2000;//20xx
    ehp->toc.month = atoi(line.substr(6, 2).c_str());
    ehp->toc.day = atoi(line.substr(9, 2).c_str());
    ehp->toc.hour = atoi(line.substr(12, 2).c_str());
    ehp->toc.minute = atoi(line.substr(15, 2).c_str());
    ehp->toc.second = atof(line.substr(18, 4).c_str());


    ehp->a0 = str2f(line.substr(22, 19));
    ehp->a1 = str2f(line.substr(41, 19));
    ehp->a2 = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 2
    ehp->IODE = str2f(line.substr(3, 19));
    ehp->Crs = str2f(line.substr(22, 19));
    ehp->delta_n = str2f(line.substr(41, 19));
    ehp->M0 = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 3
    ehp->Cuc = str2f(line.substr(3, 19));
    ehp->e = str2f(line.substr(22, 19));
    ehp->Cus = str2f(line.substr(41, 19));
    ehp->sqrt_a = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 4
    ehp->toe = str2f(line.substr(3, 19));
    ehp->Cic = str2f(line.substr(22, 19));
    ehp->Omg0 = str2f(line.substr(41, 19));
    ehp->Cis = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 5
    ehp->I0 = str2f(line.substr(3, 19));
    ehp->Crc = str2f(line.substr(22, 19));
    ehp->w = str2f(line.substr(41, 19));
    ehp->Omg0dot = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 6
    ehp->I0dot = str2f(line.substr(3, 19));
    ehp->codeL2 = str2f(line.substr(22, 19));
    ehp->GPSweek = str2f(line.substr(41, 19));
    ehp->L2flag = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 7
    ehp->accuracy = str2f(line.substr(3, 19));
    ehp->health = str2f(line.substr(22, 19));
    ehp->TGD = str2f(line.substr(41, 19));
    ehp->IODC = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 8
    ehp->trans_time_of_message = str2f(line.substr(3, 19));
    ehp->fit_interval = str2f(line.substr(22, 19));

    m[ehp->prn].push_back(ehp);
}


void jump_entry(std::ifstream& ifs, int n){

    for (int i = 0; i < n; i++){
        std::string s;
        std::getline(ifs, s);
    }
}


void read_brdc_sbas_entry(std::ifstream& ifs,EhpemerisMap& m){

}


// abstract start
GNSSFile::GNSSFile():_path(NULL)
{
}


GNSSFile::GNSSFile(const std::string& path):_path(path){}


GNSSFile::GNSSFile(const char* path):_path(std::string(path)){}
// abstract end



int BRDCFile::open(const std::string& path){

    this->_path = path;
    std::ifstream ifs(path.c_str());
    if (!ifs.is_open()) {
        return 0;
    }


    std::getline(ifs, _line);
    this->_version = trim(_line.substr(0, 10));

    this->_satellie_system = _line[40] == ' ' ? 'G': _line[40];

    assert(this->_satellie_system != 'R');
    // head
    while(trim(_line) != "END OF HEADER")
        std::getline(ifs, _line);


    // for each entry
    while(std::getline(ifs, _line)){
        std::cout << _line << "\n";
        if (this->_version == "2")
        {
            if (this->_satellie_system == 'G')
                read_brdc_gps_like_entry(ifs, this->_map, _line);
            else
                return 2;// unknown ehpemeris
        }
        else if (this->_version == "3.04")
        {
            switch (_line[0])
            {
            case('G'):
            case('E'):
            case('C'):
            case('I'):
            case('J'):
                read_brdc_gps_like_entry(ifs, this->_map, _line);
                break;
            case('S'):
                printf("Not implemented yet: %c\n", 'S');
                jump_entry(ifs, 3);
                break;
            case('R'):
                printf("Not implemented yet: %c\n", 'R');
                jump_entry(ifs, 3);
                break;
            }
        }
        else // unknown version
        {
            return -1;
        }

    }

    return 0;
}



PositionPtr BRDCFile::compute(std::string prn, const Date& date){
    assert(false);

}



void BRDCFile::writeSP3(const std::string& path){
    assert(false);
}

}
