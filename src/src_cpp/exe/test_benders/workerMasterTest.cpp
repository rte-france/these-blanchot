#include "catch2.hpp"
#include "WorkerMaster.h"
#include "define_datas.hpp"


TEST_CASE("Debug worker master", "[debug]") {

	// =======================================================================================
	// Default constructor, worker's one
	WorkerMasterPtr worker = std::make_shared<WorkerMaster>();
	REQUIRE(worker->_is_master == true);
	REQUIRE(worker->_solver == nullptr);

	worker.reset();

	// =======================================================================================
	// Full constructor
	Str2Int varmap;
	varmap["1"] = 1;
	varmap["2"] = 2;
	varmap["3"] = 3;

	SolverFactory factory;
	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");
	DblVector neededObj;
	DblVector actualObj;

	auto inst = GENERATE(NET_MASTER, MIP_TOY, MULTIKP);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;

		// Idem to worker
		
		for (auto const& solver_name : factory.get_solvers_list()) {

			// Testing 3 differents slave_weights 0, 1.0, and 10
			for (auto const& nslaves : IntVector({ 10, 20, 30})) {


				worker = std::make_shared<WorkerMaster>(varmap, instance_path,
					opt, solver_name, nslaves);

				//1. varmap
				REQUIRE(worker->_name_to_id == varmap);

				for (auto const& kvp : varmap) {
					REQUIRE(kvp.first == worker->_id_to_name[kvp.second]);
				}

				REQUIRE(worker->_solver->get_ncols() == datas[inst]._ncols + 1 + nslaves);
				REQUIRE(worker->_solver->get_nrows() == datas[inst]._nrows + 1);

				//check cols
				actualObj.clear();
				neededObj.clear();
				neededObj.resize(nslaves + 1, 0.0);
				neededObj[0] = 1.0;
				actualObj.resize(worker->_solver->get_ncols());
				worker->_solver->get_obj(actualObj.data(), 0, worker->_solver->get_ncols() - 1);
				for (int i(0); i < worker->_solver->get_ncols(); i++) {
					if (i < datas[inst]._ncols) {
						REQUIRE(actualObj[i] == datas[inst]._obj[i]);
					}
					else {
						REQUIRE(actualObj[i] == neededObj[i - datas[inst]._ncols]);
					}
				}
				actualObj.clear();
				neededObj.clear();
			}
		}
	}
}

TEST_CASE("WorkerMaster constructor", "[wrk-master][wrk-master-init]") {
	// =======================================================================================
	// Default constructor, worker's one
	WorkerMasterPtr worker = std::make_shared<WorkerMaster>();
	REQUIRE(worker->_is_master == true);
	REQUIRE(worker->_solver == nullptr);

	worker.reset();

	// =======================================================================================
	// Full constructor
	Str2Int varmap;
	varmap["1"] = 1;
	varmap["2"] = 2;
	varmap["3"] = 3;

	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");
	DblVector neededObj;
	double lb = -1e10;
	double ub = 1e20;
	DblVector lbs;
	DblVector ubs;
	DblVector o;
	IntVector mstart;
	IntVector mind;
	DblVector matval;
	int nreturned;

	SolverFactory factory;

	auto inst = GENERATE(MULTIKP, NET_MASTER, MIP_TOY);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;
		std::cout << instance_path << std::endl;
		
		for (auto const& solver_name : factory.get_solvers_list()) {

			for (int i(1); i < 4; i++) {
				int toto = 0;
				worker = std::make_shared<WorkerMaster>(varmap, datas[inst]._path, opt,
					solver_name, 10*i);

				//1. varmap
				REQUIRE(worker->_name_to_id == varmap);
				for (auto const& kvp : varmap) {
					REQUIRE(kvp.first == worker->_id_to_name[kvp.second]);
				}

				REQUIRE(worker->_solver->get_ncols() == datas[inst]._ncols + 1 + 10*i);
				REQUIRE(worker->_solver->get_nrows() == datas[inst]._nrows + 1);

				//check cols
				neededObj.resize(10 * i + 1, 0.0);
				neededObj[0] = 1.0;
				o.resize(worker->_solver->get_ncols());
				worker->_solver->get_obj(o.data(), 0, worker->_solver->get_ncols() - 1);
				for (int i(0); i < worker->_solver->get_ncols() - 1; i++) {
					if (i < datas[inst]._ncols) {
						REQUIRE(o[i] == datas[inst]._obj[i]);
					}
					else {
						REQUIRE(o[i] == neededObj[i - datas[inst]._ncols]);
					}
				}
				o.clear();
				neededObj.clear();

				
				lbs.resize(10 * i + 1);
				ubs.resize(10 * i + 1);
				worker->_solver->get_lb(lbs.data(), datas[inst]._ncols, worker->_solver->get_ncols() - 1);
				worker->_solver->get_ub(ubs.data(), datas[inst]._ncols, worker->_solver->get_ncols() - 1);
				REQUIRE(lbs == DblVector(10 * i + 1, lb));
				REQUIRE(ubs == DblVector(10 * i + 1, ub));
				lbs.clear();
				ubs.clear();
				
				// check rows
				mstart.resize(2);
				mind.resize(10 * i + 1);
				matval.resize(10 * i + 1);
				worker->_solver->get_rows(mstart.data(), mind.data(), matval.data(), 2 * 10 * i + 1, &nreturned,
					datas[inst]._nrows, datas[inst]._nrows);
				REQUIRE(nreturned == 10 * i + 1);

				mstart.clear();
				mind.clear();
				matval.clear();

				//worker->free();
				worker.reset();
			}
		}
	}
}

/*
TEST_CASE("WorkerMaster getters", "[wrk-master][wrk-master-get]") {


}

TEST_CASE("WorkerMaster add cuts", "[wrk-master][wrk-master-addcuts]") {

}

TEST_CASE("WorkerMaster del cuts", "[wrk-master][wrk-master-delcuts]") {

}

TEST_CASE("WorkerMaster bound alpha", "[wrk-master][wrk-master-alpha]") {

}*/