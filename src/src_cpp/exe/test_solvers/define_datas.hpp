#pragma once

#include <vector>
#include <string>

/* Contains all the data to check the results of the tests on an instance*/
class InstanceData {
public:
    std::string _path;
    int _nvars;
    int _nintegervars;
    int _ncols;
    int _nelems;
    std::vector<double> _obj;
    std::vector<double> _matval;
    std::vector<int> _mind;
    std::vector<int> _mstart;
    std::vector<double> _rhs;
    std::vector<char> _coltypes;
    std::vector<char> _rowtypes;
    double _optval;
    std::vector<double> _primals;
    std::vector<double> _duals;
    std::vector<double> _slacks;
    std::vector<double> _reduced_costs;
};

enum INSTANCES {
    MIP_TOY
};

typedef std::vector<InstanceData> AllDatas;

void fill_datas(AllDatas& datas);