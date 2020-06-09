#include "Benders.h"


Benders::~Benders() {
}

/*!
*  \brief Constructor of class Benders
*
*  Method to build a Benders element, initializing each problem from a list
*
*  \param problem_list : map linking each problem name to its variables and their ids
*
*  \param options : set of options fixed by the user 
*/
Benders::Benders(CouplingMap const & problem_list, BendersOptions const & options, 
	SMPSData const& smps_data) : _options(options) {

	// 1. On fixe la seed
	if (_options.SEED != -1) {
		std::srand(options.SEED);
	}
	else {
		std::srand((unsigned)time(NULL));
	}

	if (!problem_list.empty()) {

		_data.nslaves = _options.SLAVE_NUMBER;
		if (_data.nslaves < 0) {
			_data.nslaves = problem_list.size() - 1;
		}

		auto it(problem_list.begin());
		
		auto const it_master = problem_list.find(_options.MASTER_NAME);
		Str2Int const & master_variable(it_master->second);
		std::string slave_path;

		// One realisation is a line in a MPS file, with two keys :
		// Either : RIGHT ROWNAME for a RHS
		// Or : COLNAME ROWNAME for a matrix element
		// and a value associated to this element
		
		
		//StrPair2Dbl realisation;
		StrPairVector keys;
		DblVector values;
		
		IntVector real_counter;
		long int nbr_real = 1;
		/*for (auto const& kvp : smps_data._rd_entries) {
			real_counter[kvp.first] = 0;
			nbr_real *= kvp.second.size();
		}*/
		for (int i(0); i < smps_data.nbr_entries(); i++) {
			real_counter.push_back(0);
		}
		if (_options.SLAVE_NUMBER != -1) {
			smps_data.go_to_next_realisation(real_counter, _options);
		}

		double proba;

		// On cree un SP fictif (lecture mps) et on le copie ensuite
		WorkerPtr slave_fictif;
		if (options.DATA_FORMAT == "SMPS") {
			slave_fictif = WorkerPtr(new Worker());
			slave_fictif->init(it->second, _options.get_slave_path("slave_init"), _options.SOLVER);
		}

		for(int i(0); i < _data.nslaves; ++it) {

			if (it != it_master) {

				proba = smps_data.find_rand_realisation_lines(keys, values, real_counter);
				if (_options.SLAVE_NUMBER != -1) {
					proba = 1.0 / _data.nslaves;
				}

				_problem_to_id[it->first] = i;
				if (options.DATA_FORMAT == "DECOMPOSED") {
					_map_slaves[it->first] = WorkerSlavePtr(new WorkerSlave(it->second, _options.get_slave_path(it->first),
						_options.slave_weight(_data.nslaves, it->first), _options));
				}
				else if (options.DATA_FORMAT == "SMPS") {
					_map_slaves[it->first] = WorkerSlavePtr(new WorkerSlave(it->second, _options.get_slave_path("slave_init"),
						proba, _options, keys, values, slave_fictif));
					if (i + 1 < _data.nslaves) {
						smps_data.go_to_next_realisation(real_counter, _options);
					}
				}
				_slaves.push_back(it->first);
				keys.clear();
				values.clear();
				i++;
			}
		}

		_master.reset(new WorkerMaster(master_variable, _options.get_master_path(), _options, _data.nslaves));

		if (_master->get_n_integer_vars() > 0) {
			if (options.ALGORITHM == "IN-OUT") {
				std::cout << "ERROR : IN-OUT algorithm can not be used with integer problems." << std::endl;
				std::cout << "Please set alorithm to BASE." << std::endl;
				std::exit(0);
			}
		}
	}
}


/*!
*  \brief Method to free the memory used by each problem
*/
void Benders::free() {
	_master->free();
	for (auto & ptr : _map_slaves)
		ptr.second->free();
}

/*!
*  \brief Build Slave cut and store it in the Benders trace
*
*  Method to build Slaves cuts, store them in the Benders trace and add them to the Master problem
*
*/
void Benders::build_cut() {
	SlaveCutPackage slave_cut_package;
	AllCutPackage all_package;
	Timer timer_slaves;
	if (_options.ALGORITHM == "ENHANCED_MULTICUT") {
		_data.slave_status = get_random_slave_cut(slave_cut_package, _map_slaves, _slaves, _options, _data, _problem_to_id);
	}
	else if(_options.ALGORITHM == "BASE" || _options.ALGORITHM == "IN-OUT"){
		_data.slave_status = get_slave_cut(slave_cut_package, _map_slaves, _options, _data);
	}
	else {
		std::cout << "ALGORHTME NON RECONNU" << std::endl;
		std::exit(0);
	}
	_data.timer_slaves = timer_slaves.elapsed();
	all_package.push_back(slave_cut_package);
	build_cut_full(_master, all_package, _problem_to_id, _slave_cut_id, _all_cuts_storage, _data, _options);
}

/*!
*  \brief Run Benders algorithm
*
*  Method to run Benders algorithm
*
* \param stream : stream to print the output
*/
void Benders::run(std::ostream & stream) {
	
	Timer timer;

	init_log(stream, _options.LOG_LEVEL, _options);
	for (auto const & kvp : _problem_to_id) {
		_all_cuts_storage[kvp.first] = SlaveCutStorage();
	}
	
	init(_data, _options);

	

	if (_options.ALGORITHM == "ENHANCED_MULTICUT") {
		master_loop(stream);
	}

	while (!_data.stop) {
		if (_options.ALGORITHM == "BASE" || _options.ALGORITHM == "IN-OUT") {
			classic_iteration(stream);
		}
		else if (_options.ALGORITHM == "ENHANCED_MULTICUT") {
			enhanced_multicut_iteration(stream);
		}
		else {
			std::cout << "ERROR : UNKNOWN ALGORITHM " << std::endl;
			std::exit(0);
		}
	}
	print_solution(stream, _data.x_cut, true, _data.global_prb_status, _options.PRINT_SOLUTION);

	std::cout << "Computation time : " << timer.elapsed() << std::endl;
}


/*!
*  \brief Perform one iteration of classic Benders decomposition
*
*  Perform one iteration of classic Benders decomposition (ALGORITHM = BASE or IN-OUT)
*
* \param stream : stream to print the output
*/
void Benders::classic_iteration(std::ostream& stream) {
	Timer timer_master;
	++_data.it;

	reset_iteration_data(_data, _options);
	get_master_value(_master, _data, _options);
	_data.ub = 0;

	compute_x_cut(_options, _data);
	build_cut();
	compute_ub(_master, _data);
	
	update_in_out_stabilisation(_master, _data, _options);

	update_best_ub(_data.best_ub, _data.ub, _data.bestx, _data.x0);
	_data.timer_master = timer_master.elapsed();
	print_log(stream, _data, _options.LOG_LEVEL, _options);
	_data.stop = stopping_criterion(_data, _options);
}

/*!
*  \brief Perform one iteration of enhanced multicut Benders decomposition
*
*  Perform one iteration of enhanced multicut Benders decomposition (ALGORITHM = ENHANCED_MULTICUT)
*
* \param stream : stream to print the output
*/

void Benders::enhanced_multicut_iteration(std::ostream& stream) {
	Timer timer_master;
	++_data.it;

	reset_iteration_data(_data, _options);

	if (_data.has_cut == true) {
		_data.has_cut = false;
		_data.n_slaves_no_cut = 0;
		_data.misprices = 0;

		get_master_value(_master, _data, _options);
		set_slaves_order(_data, _options);
		compute_x_cut(_options, _data);
	}

	build_cut();

	_data.timer_master = timer_master.elapsed();
	
	if (_data.n_slaves_no_cut == _data.nslaves) {
		compute_ub(_master, _data);
		_data.has_cut = true;
	}
	_data.stop = stopping_criterion(_data, _options);

	if (_data.it % _options.LOG_NUMBER_ITE == 0 || _data.stop) {
		print_log(stream, _data, _options.LOG_LEVEL, _options);
	}	
}

void Benders::master_loop(std::ostream& stream) {

	double ub_memory = 0;
	double beta = 1.0 - (1.0 / (1.0 * _data.nslaves));
	double grad;
	double last_grad = 0;

	double last_ub = 0;

	double lbk_2 = 0.0;
	double lbk_1 = 0.0;
	_data.n_slaves_solved = 1;

	int baisse = 0;
	int hausse = 0;

	while (!_data.stop) {

		if (_data.it > 1) {
			lbk_2 = lbk_1;
			lbk_1 = _data.lb;
		}

		// 1. resolution of master problem
		get_master_value(_master, _data, _options);

		/*if (_data.it == 0) {
			_data.x_cut = _data.x0;
			_data.x_stab = _data.x0;
		}*/
		_data.has_cut = false;
		
		// 2. Choosing new order of subpoblems
		//set_slaves_order(_data, _options);

		// 3. reset misprice information
		_data.misprices = 0;

		// 4. settin in-point to last separation point
		_data.x_stab = _data.x_cut;

		// 5. cutting loop
		separation_loop(stream);

		// 6. update stab
		if (_options.ALPHA_STRAT == "DYNAMIQUE") {

			if (_data.it == 1) {
				ub_memory = _data.ub;
				_data.best_ub = _data.ub;
				last_ub = _data.ub;
				lbk_1 = _data.lb;
			}
			else {
				//lbk_2 = lbk_1;
				//lbk_1 = _data.lb;
			}

			bool print = 0;
			if (print) {
				std::cout << "     " << _data.step_size
					<< "   " << lbk_2
					<< "   " << lbk_1
					<< "   " << _data.lb
					<< "   " << _data.lb - lbk_1
					<< "   " << lbk_1 - lbk_2
					<< std::endl;
			}

			// 1. si on ralentit

			if (_data.lb - lbk_1 <= lbk_1 - lbk_2 + 1e-6) {
				_data.step_size = std::min(1.0, _data.step_size / (1.0 - 0.03));
				hausse += 1;
			}
			else {
				baisse += 1;
				_data.step_size = std::max(0.1, _data.step_size * (1.0 - 0.03));
			}

			/*if (_data.ub < last_ub) {
				_data.step_size = std::min(1.0, _data.step_size / (1.0 - 0.3 * (float(_data.n_slaves_solved) / float(_data.nslaves))));
			}
			else {
				_data.step_size = std::max(0.01, _data.step_size * (1.0 - 0.3 * (float(_data.n_slaves_solved) / float(_data.nslaves))));
			}*/

			last_grad = std::max(0.0, last_ub - _data.ub);

			_data.best_ub = _data.ub;
			last_ub = _data.ub;

			grad = ub_memory;
			ub_memory = beta * ub_memory + (1 - beta) * _data.ub;
			grad -= ub_memory;
			//std::cout << grad << std::endl;
		}
		else if (_options.ALPHA_STRAT == "STATIQUE") {
			_data.step_size = _options.STEP_SIZE;
		}
		else {
			std::cout << "BAD STRATEGY" << std::endl;
			std::exit(0);
		}
		

		if (_data.stop) {
			print_log(stream, _data, _options.LOG_LEVEL, _options);
		}

	}
}

void Benders::separation_loop(std::ostream& stream)
{
	while ( _data.has_cut == false ) {

		_data.n_slaves_solved = 0;
		set_slaves_order(_data, _options);

		// 1. Compute separation point
		compute_x_cut(_options, _data);

		// 2. Compute difference of first stage solutions objectives
		compute_separation_point_cost(_master, _data, _options);
		compute_epsilon_x(_master, _options, _data);

		// 3. Reset resolution indicator for point x_cut
		_data.n_slaves_no_cut = 0;

		// 4. Resolution of subproblems in x_cut
		optimality_loop(stream);

		// 5. stopping criterion
		_data.stop = stopping_criterion(_data, _options);

		// 6. udpate stab value
		if (_data.has_cut == false) {
			_data.misprices += 1;
			_data.step_size = std::min(1.0, _data.step_size * (1.0 + (1.0 / _data.misprices)));
			std::cout << "MISPRICE : " << _data.step_size << std::endl;
		}
	}
}

void Benders::optimality_loop(std::ostream& stream)
{
	do {

		reset_iteration_data(_data, _options);

		build_cut();

		if (_data.it % _options.LOG_NUMBER_ITE == 0 || _data.stop) {
			print_log(stream, _data, _options.LOG_LEVEL, _options);
		}

		++_data.it;

	} while (_data.stay_in_x_cut);

	_data.ub /= float(_data.n_slaves_solved) / float(_data.nslaves);
	_data.ub += _data.invest_separation_cost;
}
