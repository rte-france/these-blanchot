#include "define_datas.hpp"


void fill_datas(AllDatas& datas) {

    datas.clear();

    //==================================================================
    //1. mip toy
    InstanceData miptoy = InstanceData();
    miptoy._path = "../../data_test/mip_toy_prob.mps";
    miptoy._nvars = 2;
    miptoy._nintegervars = 2;
    miptoy._ncols = 2;
    miptoy._nelems = 2;
    miptoy._obj = { -5.0, -4.0 };
    miptoy._matval = { 1.0, 1.0, 10.0, 6.0 };
    miptoy._mind = { 0, 1, 0, 1 };
    miptoy._mstart = { 0, 2, 4 };
    miptoy._rhs = { 5.0, 45.0 };
    miptoy._coltypes = { 'I', 'I' };
    miptoy._rowtypes = { 'L', 'L' };
    miptoy._optval = -23.0;
    miptoy._primals = { 3, 2 };
    miptoy._duals = {};
    miptoy._slacks = { 0, 1 };
    miptoy._reduced_costs = {};

    datas.push_back(miptoy);
}