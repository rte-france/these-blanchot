#include "catch2.hpp"
#include "Worker.h"
#include "define_datas.hpp"

TEST_CASE("Test Worker.h") {
	
	//========================================================================================
	// 1. declaration et initialisation
	Worker worker;
	REQUIRE(worker._is_master == false);
	REQUIRE(worker._solver == nullptr);

	Str2Int varmap;
	varmap["1"] = 1;
	varmap["2"] = 2;
	varmap["3"] = 3;
	varmap["4"] = 4;
	varmap["5"] = 5;

	AllDatas datas;
	fill_datas(datas);
	std::string instance_path = datas[MIP_TOY]._path;
	
	SolverFactory factory;
	for (auto const& solver_name : factory.get_solvers_list()) {
		worker.init(varmap, instance_path, solver_name);

		REQUIRE(worker._name_to_id == varmap);

		for (auto const& kvp : varmap) {
			REQUIRE(kvp.first == worker._id_to_name[kvp.second]);
		}
	}
}

TEST_CASE("Test solving worker") {

	AllDatas datas;
	fill_datas(datas);

	//========================================================================================
	// 1. declaration et initialisation
	Worker worker;
	Str2Int varmap;

	auto inst = GENERATE(MIP_TOY, MULTIKP);
	SECTION("Read and solving through Worker") {

		SolverFactory factory;
		for (auto const& solver_name : factory.get_solvers_list()) {

			worker._is_master = true;

			std::string instance_path = datas[inst]._path;
			worker.init(varmap, instance_path, solver_name);

			int lp_status = 0;
			worker.solve(lp_status);
			bool success = false;
			for (auto stat : datas[inst]._status_int) {
				if (stat == lp_status) {
					SUCCEED();
					success = true;
					break;
				}
			}
			if (!success) {
				FAIL();
			}

			success = false;
			for (auto stat : datas[inst]._status) {
				if (stat == worker._solver->SOLVER_STRING_STATUS[lp_status]) {
					SUCCEED();
					success = true;
					break;
				}
			}
			if (!success) {
				FAIL();
			}

			double val(0);
			worker.get_value(val);
			REQUIRE(val == datas[inst]._optval);

			worker.free();
		}
	}

		
}

TEST_CASE("Mid size LP to check simplex iterations") {
	Worker worker;
	Str2Int varmap;

	SolverFactory factory;
	for (auto const& solver_name : factory.get_solvers_list()) {

		worker._is_master = true;

		std::string instance_path = "../../data_test/pgp2.mps";
		worker.init(varmap, instance_path, solver_name);

		int lp_status = 0;
		worker.solve(lp_status);

		REQUIRE(lp_status == OPTIMAL);

		int simplexIter = 0;
		worker.get_simplex_ite(simplexIter);
		REQUIRE(simplexIter > 0);

		worker.free();
		REQUIRE(worker._solver == nullptr);
	}
	
	
}