#include "catch2.hpp"
#include "WorkerSlave.h"
#include "define_datas.hpp"

TEST_CASE("Worker Slave instanciation", "[wrk-slave][wrk-slave-init]") {

	// =======================================================================================
	// Default constructor, worker's one
	WorkerSlavePtr worker;
	REQUIRE(worker->_is_master == false);
	REQUIRE(worker->_solver == nullptr);

	worker.reset();

	// =======================================================================================
	// Full constructor
	Str2Int varmap;
	varmap["1"] = 1;
	varmap["2"] = 2;
	varmap["3"] = 3;
	varmap["4"] = 4;
	varmap["5"] = 5;

	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");
	DblVector neededObj;
	DblVector actualObj;

	auto inst = GENERATE(MIP_TOY, MULTIKP);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;

		// Idem to worker
		SolverFactory factory;
		for (auto const& solver_name : factory.get_solvers_list()) {

			// Testing 3 differents slave_weights 0, 1.0, and 10
			for (auto const& slave_wieght : DblVector({ 0.0, 1.0, 10.0 })) {

				worker = std::make_unique<WorkerSlave>(varmap, instance_path,
					slave_wieght, opt, solver_name);

				//1. varmap
				REQUIRE(worker->_name_to_id == varmap);

				for (auto const& kvp : varmap) {
					REQUIRE(kvp.first == worker->_id_to_name[kvp.second]);
				}

				//2. obj
				neededObj.clear();
				neededObj.resize(datas[inst]._ncols);
				for (int i(0); i < datas[inst]._obj.size(); i++) {
					neededObj[i] = datas[inst]._obj[i] * slave_wieght;
				}
				

				actualObj.clear();
				actualObj.resize(worker->_solver->get_ncols());
				worker->_solver->get_obj(actualObj.data(), 0, worker->_solver->get_ncols());
				REQUIRE(actualObj == neededObj);
			}

		}
	}
}

TEST_CASE("Worker Slave methods", "[wrk-slave]") {

}