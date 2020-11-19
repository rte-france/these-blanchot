#include "define_datas.hpp"


void fill_datas(AllDatas& datas) {

    datas.clear();

    //==================================================================
    //1. mip toy
    InstanceData miptoy = InstanceData();
    miptoy._path = "../../data_test/mip_toy_prob.mps";
    miptoy._ncols = 2;
    miptoy._nintegervars = 2;
    miptoy._nrows = 2;
    miptoy._nelems = 4;
    miptoy._obj = { -5.0, -4.0 };

    miptoy._matval = { 1.0, 1.0, 10.0, 6.0 };
    miptoy._mind = { 0, 1, 0, 1 };
    miptoy._mstart = { 0, 2, 4 };

    miptoy._matval_delete = { 10.0, 6.0 };
    miptoy._mind_delete = { 0, 1 };
    miptoy._mstart_delete = { 0, 2 };

    miptoy._rhs = { 5.0, 45.0 };
    miptoy._lb = { 0.0, 0.0 };
    miptoy._ub = { 1e20, 1e20 };
    miptoy._coltypes = { 'I', 'I' };
    miptoy._rowtypes = { 'L', 'L' };
    miptoy._optval = -23.0;
    miptoy._primals = { 3, 2 };
    miptoy._duals = {};
    miptoy._slacks = { 0, 1 };
    miptoy._reduced_costs = {};

    miptoy._status = { "OPTIMAL" };
    miptoy._status_int = { OPTIMAL };

    datas.push_back(miptoy);

    //==================================================================
    //2. multi knapsack
    InstanceData multikp = InstanceData();
    multikp._path = "../../data_test/lp1.mps";
    multikp._ncols = 9;
    multikp._nintegervars = 0;
    multikp._nrows = 5;
    multikp._nelems = 15;
    multikp._obj = { 4.0, 4.0, 4.0, 2.0, 2.0, 2.0, 0.5, 0.5, 0.5 };

    multikp._matval = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    multikp._mind = { 0, 3, 6, 1, 4, 7, 2, 5, 8, 0, 1, 2, 3, 4, 5 };
    multikp._mstart = { 0, 3, 6, 9, 12, 15 };

    multikp._matval_delete = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
    multikp._mind_delete = { 1, 4, 7, 2, 5, 8, 0, 1, 2, 3, 4, 5 };
    multikp._mstart_delete = { 0, 3, 6, 9, 12 };

    multikp._rhs = { 12.0, 7.1, 10.0, 5.5, 8.3 };
    multikp._lb = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    multikp._ub = { 1e20, 1e20, 1e20, 1e20, 1e20, 1e20, 1e20, 1e20, 1e20 };
    multikp._coltypes = { 'C', 'C', 'C', 'C', 'C', 'C','C', 'C', 'C' };
    multikp._rowtypes = { 'L', 'L', 'L', 'L', 'L' };
    multikp._optval = 46.25;
    multikp._primals = { 0.0, 0.0, 5.5, 8.3, 0.0, 0.0, 3.7, 7.1, 4.5 };
    multikp._duals = {};
    multikp._slacks = { };
    multikp._reduced_costs = {};

    multikp._status = { "OPTIMAL" };
    multikp._status_int = { OPTIMAL };

    datas.push_back(multikp);

    //==================================================================
    //3. unbounded
    InstanceData unbd = InstanceData();
    unbd._path = "../../data_test/unbounded.mps";
    unbd._ncols = 2;
    unbd._nintegervars = 0;
    unbd._nrows = 2;
    unbd._nelems = 3;
    unbd._obj = { 4.0, 2.67 };

    unbd._matval = { -1.0, 64.0, -21.0 };
    unbd._mind = { 0, 1, 1 };
    unbd._mstart = { 0, 2, 3 };

    unbd._matval_delete = { -21.0 };
    unbd._mind_delete = { 1 };
    unbd._mstart_delete = { 0, 1 };

    unbd._rhs = { 12.0, -3.0 };
    unbd._lb = { 0.0, 0.0 };
    unbd._ub = { 1e20, 1e20 };
    unbd._coltypes = { 'C', 'C' };
    unbd._rowtypes = { 'L', 'G' };
    unbd._optval = -1;
    unbd._primals = { };
    unbd._duals = { };
    unbd._slacks = { };
    unbd._reduced_costs = { };

    unbd._status = { "UNBOUNDED", "INForUNBOUND" };
    unbd._status_int = { UNBOUNDED, INForUNBOUND };

    datas.push_back(unbd);

    //==================================================================
    //3. unbounded
    InstanceData infeas = InstanceData();
    infeas._path = "../../data_test/infeas.mps";
    infeas._ncols = 2;
    infeas._nintegervars = 0;
    infeas._nrows = 2;
    infeas._nelems = 4;
    infeas._obj = { 4.0, -2.1 };

    infeas._matval = { 2.0, 1.0, 1.0, 1.0 };
    infeas._mind = { 0, 1, 0, 1 };
    infeas._mstart = { 0, 2, 4 };

    infeas._matval_delete = { 1.0, 1.0 };
    infeas._mind_delete = { 0, 1 };
    infeas._mstart_delete = { 0, 2 };

    infeas._rhs = { 2.0, 4.0 };
    infeas._lb = { 0.0, 0.0 };
    infeas._ub = { 1e20, 1e20 };
    infeas._coltypes = { 'C', 'C' };
    infeas._rowtypes = { 'L', 'G' };
    infeas._optval = -1;
    infeas._primals = { };
    infeas._duals = { };
    infeas._slacks = { };
    infeas._reduced_costs = { };

    infeas._status = { "INFEASIBLE", "INForUNBOUND" };
    infeas._status_int = { INFEASIBLE, INForUNBOUND };

    datas.push_back(unbd);

}