#pragma once

#include <vector>
#include <string>
#include "SolverAbstract.h"

/* Contains all the data to check the results of the tests on an instance*/
class InstanceData {
public:
    std::string _path;
    int _ncols;
    int _nintegervars;
    int _nrows;
    int _nelems;
    std::vector<double> _obj;

    std::vector<double> _matval;
    std::vector<int> _mind;
    std::vector<int> _mstart;

    std::vector<double> _matval_delete;
    std::vector<int> _mind_delete;
    std::vector<int> _mstart_delete;

    std::vector<double> _rhs;
    std::vector<char> _coltypes;
    std::vector<char> _rowtypes;
    std::vector<double> _lb;
    std::vector<double> _ub;
    double _optval;
    std::vector<double> _primals;
    std::vector<double> _duals;
    std::vector<double> _slacks;
    std::vector<double> _reduced_costs;

    std::vector<std::string> _status;
    std::vector<int> _status_int;

    std::string first_col_name;
    std::string first_row_name;
};

enum INSTANCES {
    MIP_TOY,
    MULTIKP,
    UNBD_PRB,
    INFEAS_PRB,
    NET_MASTER,
    NET_SP1,
    NET_SP2
};

typedef std::vector<InstanceData> AllDatas;

void fill_datas(AllDatas& datas);