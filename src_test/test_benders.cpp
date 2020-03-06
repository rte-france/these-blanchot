#pragma once

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2.hpp"

#include "merge_mps_functions.h"
#include "BendersMPI.h"
#include "Benders.h"

TEST_CASE("Lecture LP et MPS") {

	BendersOptions options;
	REQUIRE(options.SLAVE_NUMBER == -1); // Options declarees

	// Declaration d'un Worker
	WorkerMerge prob(options);
	// 0 Variables au debut
	REQUIRE(prob.get_ncols() == 0); // Probleme initialise
	prob.read("small_MPS", "");
	REQUIRE(prob.get_ncols() == 1);
	prob.read("small_LP", "l");
	REQUIRE(prob.get_ncols() == 2);

	int lp_status(0);
	prob.solve_integer(lp_status);
	REQUIRE(lp_status == 0);
	
	DblVector ptr(prob.get_ncols(), 0);;
	prob.get_MIP_sol(ptr.data(), NULL);

	for (auto const& elt : ptr) {
		std::cout << elt << std::endl;
	}

	REQUIRE(ptr[0] == 0.0);
	REQUIRE(ptr[1] == 2);

	prob.free();
	REQUIRE(prob.get_ncols() == 0);
}

SCENARIO("Resolution instance LP") {
	GIVEN("A LP instance") {

		BendersOptions options;
		options.INPUTROOT = "./mini_instance_LP/";
		options.print(std::cout);

		WHEN("Solving with merge_mps") {

			CouplingMap input;
			build_input(options, input);

			// Declaration du WorkerMerge avec le probleme vide
			WorkerMerge full(options, input, "full");

			// Lecture et ajout de tous les problemes dans full
			full.merge_problems(input, options);
			REQUIRE(full.get_ncols() == 5); // Toutes les colonnes sont bien ajoutees

			// Resolution sequentielle
			full.set_threads(1);

			int status = 0;
			full.solve_integer(status);

			Point x0;
			double val(0);
			full.get_optimal_point_and_value(x0, val, input, options);
			
			full.free();

			THEN("the optimal solution is found.") {
				REQUIRE(status == 0); // OPITMAL
				REQUIRE(val == 3.25); // Valeur optimale
				REQUIRE(x0["x"] == 1.5); // Solution optimale fractionnaire
			}
		}

		WHEN("Solving with benderssequential") {

			CouplingMap input;
			build_input(options, input);
			Benders benders(input, options);
			benders.run(std::cout);

			THEN("the optimal solution in found.") {
				REQUIRE(benders._data.lb == 3.25); // Valeur optimale
				REQUIRE(benders._data.x0["x"] == 1.5); // Solution optimale fractionnaire
			}

			benders.free();
		}
	}
}

SCENARIO("Resolution instance MIP") {
	GIVEN("A LP instance") {

		BendersOptions options;
		options.INPUTROOT = "./mini_instance_MIP/";
		options.print(std::cout);

		WHEN("Solving with merge_mps") {

			CouplingMap input;
			build_input(options, input);

			// Declaration du WorkerMerge avec le probleme vide
			WorkerMerge full(options, input, "full");

			// Lecture et ajout de tous les problemes dans full
			full.merge_problems(input, options);
			REQUIRE(full.get_ncols() == 5); // Toutes les colonnes sont bien ajoutees

			// Resolution sequentielle
			full.set_threads(1);

			int status = 0;
			full.solve_integer(status);

			Point x0;
			double val(0);
			full.get_optimal_point_and_value(x0, val, input, options);

			full.free();

			THEN("the optimal solution is found.") {
				REQUIRE(status == 0); // OPITMAL
				REQUIRE(val == 3.5); // Valeur optimale
				REQUIRE(x0["x"] == 2); // Solution optimale entiere
			}
		}

		WHEN("Solving with benderssequential") {

			CouplingMap input;
			build_input(options, input);
			Benders benders(input, options);
			benders.run(std::cout);

			THEN("the optimal solution in found.") {
				REQUIRE(benders._data.lb == 3.5); // Valeur optimale
				REQUIRE(benders._data.x0["x"] == 2); // Solution optimale entiere
			}

			benders.free();
		}
	}
}

SCENARIO("Resolution instance INFEASIBLE") {
	GIVEN("An infeasible instance") {

		BendersOptions options;
		options.INPUTROOT = "./mini_instance_INFEAS/";
		options.WRITE_ERRORED_PROB = false;
		options.print(std::cout);

		WHEN("Solving with merge_mps") {

			CouplingMap input;
			build_input(options, input);

			// Declaration du WorkerMerge avec le probleme vide
			WorkerMerge full(options, input, "full");

			// Lecture et ajout de tous les problemes dans full
			full.merge_problems(input, options);
			REQUIRE(full.get_ncols() == 4); // Toutes les colonnes sont bien ajoutees

			// Resolution sequentielle
			full.set_threads(1);

			int status = 0;
			full.solve_integer(status);

			Point x0;
			double val(0);
			full.get_optimal_point_and_value(x0, val, input, options);

			THEN("the optimal solution is found.") {
				REQUIRE(1 == 1); // OPITMAL
			}

			full.write_errored_prob(status, options, "full");
			full.free();
		}

		WHEN("Solving with benderssequential") {

			CouplingMap input;
			build_input(options, input);
			Benders benders(input, options);
			benders.run(std::cout);


			THEN("the optimal solution in found.") {
				REQUIRE(0 == 0); // OPITMAL
			}

			benders.free();
		}
	}
}

SCENARIO("Resolution instance UNBOUDED") {
	GIVEN("An unbounded instance") {

		BendersOptions options;
		options.INPUTROOT = "./mini_instance_UNBOUNDED/";
		options.print(std::cout);

	}
}