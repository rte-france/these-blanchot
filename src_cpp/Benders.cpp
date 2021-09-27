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

	// 1. Fixing seed
	std::mt19937 rdgen;
	if (_options.SEED != -1) {
		std::cout << "SEED : " << _options.SEED << std::endl;
		std::srand(options.SEED);
		rdgen.seed(options.SEED);
	}
	else {
		unsigned seedTime = std::chrono::system_clock::now().time_since_epoch().count();
		rdgen.seed(seedTime);
		std::srand((unsigned)time(NULL));
	}
	std::uniform_real_distribution<double> dis(0.0, 1.0);

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
			smps_data.go_to_next_realisation(real_counter, _options, rdgen, dis);
		}

		double proba;

		// Creating one fictive subproblem to copy it in order to create every subproblem
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
						smps_data.go_to_next_realisation(real_counter, _options, rdgen, dis);
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

	// Ajout de la matrice du maitre et du RHS dans les donnees en dur
	read_master_cstr(_data, _options);

	std::cout << "ceci nest pas affiche a lecran" << std::endl;
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
	_data.timer_slave.restart();
	if (_options.ALGORITHM == "ENHANCED_MULTICUT") {
		_data.slave_status = get_random_slave_cut(slave_cut_package, _map_slaves, _slaves, _options, 
			_data, _problem_to_id);
	}
	else if(_options.ALGORITHM == "BASE" || _options.ALGORITHM == "IN-OUT"){
		_data.slave_status = get_slave_cut(slave_cut_package, _map_slaves, _options, _data);
	}
	else {
		std::cout << "ALGORHTME NON RECONNU" << std::endl;
		std::exit(0);
	}
	_data.time_slaves = _data.timer_slave.elapsed();
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

	// set numerical emphasis parameter
	numerical_emphasis(_master, _options);

	_data.timer_iter.restart();
	_data.timer_other.restart();
	_data.time_total = 0.0;

	if (_options.ALGORITHM == "ENHANCED_MULTICUT") {
		master_loop(stream);
	}
	else if (_options.ALGORITHM == "BASE" || _options.ALGORITHM == "IN-OUT") {
		classic_iteration(stream);
	}else {
		std::cout << "ERROR : UNKNOWN ALGORITHM " << std::endl;
		std::exit(0);
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
	_data.timer_master.restart();
	++_data.it;

	reset_iteration_data(_data, _options);

	get_master_value(_master, _data, _options);
	
	_data.time_master = _data.timer_master.elapsed();
	compute_x_cut(_options, _data);
	build_cut();
	compute_ub(_master, _data);
	
	update_in_out_stabilisation(_master, _data, _options);

	update_best_ub(_data.best_ub, _data.ub, _data.bestx, _data.x0);
	
	print_log(stream, _data, _options.LOG_LEVEL, _options);
	_data.stop = stopping_criterion(_data, _options);
}

void Benders::master_loop(std::ostream& stream) {

	double ub_memory = 0;
	double beta = 1.0 - (1.0 / (1.0 * _data.nslaves));

	_data.n_slaves_solved = 1;

	_data.early_termination = false;

	
	for (int melange = 0; melange < _options.N_MELANGES; melange++) {
		//std::cout << "SHUFFLE " << std::endl;
		std::random_shuffle(_data.indices.begin(), _data.indices.end());
	}

	while (!_data.stop) {

		// 1. resolution of master problem
		_data.timer_master.restart();
		get_master_value(_master, _data, _options);
		_data.time_master = _data.timer_master.elapsed();
		_data.last_time_master = _data.time_master;

		_data.has_cut = false;

		// 3. reset misprice information
		_data.misprices = 0;

		// 4. settin in-point to last separation point
		_data.x_stab = _data.x_cut;

		// 5. cutting loop
		separation_loop(stream);

		if (_data.stop) {
			print_log(stream, _data, _options.LOG_LEVEL, _options);
			if (_data.early_termination) {
				std::cout << "    EARLY TERMINATION, UNABLE TO PROGRESS : FINAL GAP = " 
					<< std::scientific << std::setprecision(8) << _data.final_gap << std::endl;
			}
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
			std::cout << "       MISPRICE : " << _data.step_size << std::endl;
		}
	}
}

void Benders::optimality_loop(std::ostream& stream)
{
	do {

		reset_iteration_data(_data, _options);

		if (_data.it > 0) {
			_data.batch_size = std::min(
				_data.nslaves - _data.n_slaves_no_cut,
				_options.BATCH_SIZE);
		}

		build_cut();

		_data.time_iter = _data.timer_iter.elapsed();
		_data.time_other = _data.time_iter - (_data.time_master + _data.time_slaves);
		_data.time_total += _data.time_iter;

		if (_data.it % _options.LOG_NUMBER_ITE == 0 || _data.stop) {
			print_log(stream, _data, _options.LOG_LEVEL, _options);
		}

		_data.time_master	= 0.0;

		_data.timer_iter.restart();
		_data.timer_other.restart();

		++_data.it;

		/*if (_data.maxsimplexiter == 0) {
			_data.nul_simplex_cnt += _options.BATCH_SIZE;

			if (_data.nul_simplex_cnt > _data.nslaves) {

				_data.ub = _data.invest_separation_cost;

				SlaveCutPackage slave_cut_package;
				AllCutPackage all_package;
				get_slave_cut(slave_cut_package, _map_slaves, _options, _data);
				all_package.push_back(slave_cut_package);
				build_cut_full(_master, all_package, _problem_to_id, _slave_cut_id, _all_cuts_storage, _data, _options);

				_data.early_termination = true;
				_data.final_gap = _data.ub - _data.lb;
			}
		}
		else {
			_data.nul_simplex_cnt = 0;
		}*/

	} while (_data.stay_in_x_cut);
	
	_data.ub /= float(_data.n_slaves_solved) / float(_data.nslaves);
	_data.ub += _data.invest_separation_cost;
}
