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
		_data.slave_weight = 52.0/_data.nslaves;
		_data.alpha_i.resize(_data.nslaves);

		auto it(problem_list.begin());
		
		auto const it_master = problem_list.find(_options.MASTER_NAME);
		Str2Int const & master_variable(it_master->second);
		std::string slave_path;

		// One realisation is a line in a MPS file, with two keys :
		// Either : RIGHT ROWNAME for a RHS
		// Or : COLNAME ROWNAME for a matrix element
		// and a value associated to this element
		
		StrPairVector keys;
		DblVector values;

		StrPairVector mean_keys;
		DblVector mean_values;
		
		// real_counter represents one outcome of a random realization
		// [0, 5, 3, 7] means we take the first outcome of the first random variable,
		// then the fifth of the second random variable ect.
		int nbr_rd_vars = smps_data.nbr_entries();
		IntVector real_counter(nbr_rd_vars, 0);
		/*for (int i(0); i < smps_data.nbr_entries(); i++) {
			real_counter.push_back(0);
		}*/
		if (_options.SLAVE_NUMBER != -1) {
			// if -1, then we take all the realisations, so no sampling is needed
			// else, in this case, we initailize the first realization here
			smps_data.go_to_next_realisation(real_counter, _options, rdgen, dis);
		}

		double proba;		

		// Creating one fictive subproblem to copy it in order to create every subproblem
		WorkerPtr slave_fictif;
		if (options.DATA_FORMAT == "SMPS") {
			slave_fictif = WorkerPtr(new Worker());
			slave_fictif->init(it->second, _options.get_slave_path("slave_init"), _options.SOLVER);
		}

		// Only to initialization of mean_value_problem elements
		// We get keys, values and proba of the current realisation
		// mean_value problem will be created at the end of the loop 
		smps_data.find_rand_realisation_lines(keys, values, real_counter);
		for (auto& v : values) { 
			mean_values.push_back(0.0); 
		}
		
		mean_keys.insert(mean_keys.end(), keys.begin(), keys.end());
		keys.clear();
		values.clear();

		// Loop on sampled realizations to create all slaves
		for(int i(0); i < _data.nslaves; ++it) {

			if (it != it_master) {

				// We get keys, values and proba of the current realisation
				proba = smps_data.find_rand_realisation_lines(keys, values, real_counter);
				if (_options.SLAVE_NUMBER != -1) {
					proba = 1.0 / _data.slave_weight;
				}

				// Update mean_values to create mean_value problem
				for (int i = 0; i < mean_values.size(); i++) {
					//std::cout << "     " << keys[i].first << "   " << keys[i].second << "    " << values[i]		 << std::endl;
					mean_values[i] += values[i] / _data.slave_weight;
				}

				// Creating actual slave problem
				_problem_to_id[it->first] = i;
				if (options.DATA_FORMAT == "DECOMPOSED") {

					_map_slaves[it->first] = WorkerSlavePtr(new WorkerSlave(it->second, _options.get_slave_path(it->first),
						_options.slave_weight(_data.slave_weight, it->first), _options));
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
		_master.reset(new WorkerMaster(master_variable, _options.get_master_path(), _options, _data.nslaves, _data.slave_weight));

		// // Creating mean value problem with no respect about encapsulation at all
		// Timer timer_init_point;
		// timer_init_point.restart();

		// _mean_value_prb = WorkerPtr(new Worker());
		// _mean_value_prb->declare_solver(_options.SOLVER, NULL);
		// _mean_value_prb->_solver->init(_options.CORFILE_NAME);
		// _mean_value_prb->_solver->read_prob(_options.CORFILE_NAME.c_str(), "MPS");
		// std::cout << _options.CORFILE_NAME.c_str() << std::endl;
		// std::cout << _mean_value_prb->get_ncols() << std::endl;
		// solve_mean_value_problem(mean_keys, mean_values);

		// std::cout << "Init point solve and get sol time : " << timer_init_point.elapsed() << std::endl;

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
	_data.timer_slave.restart();
	if (_options.ALGORITHM == "ENHANCED_MULTICUT") {
		_data.slave_status = get_random_slave_cut(slave_cut_package, _map_slaves, _slaves, _options, 
			_data, _problem_to_id);
	}
	else if(_options.ALGORITHM == "BASE" || _options.ALGORITHM == "IN-OUT" || _options.ALGORITHM == "LEVEL"){
		_data.slave_status = get_slave_cut(slave_cut_package, _map_slaves, _options, _data);
	}
	else {
		std::cout << "ALGORIHTME NON RECONNU" << std::endl;
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

	if (_options.INIT_MEAN_VALUE_SOLUTION) {
		_data.x_cut  = _x_init;
		_data.x_stab = _x_init;
		_data.ub = 0;
		
		// Resolution of every subproblem to get the cost of initial solution
		std::string true_algo = _options.ALGORITHM;
		_options.ALGORITHM = "BASE";
		build_cut();
		compute_ub(_master, _data);
		_options.ALGORITHM = true_algo;

		_data.bestx = _x_init;
		_data.best_ub = _data.ub;
	}
	else {
		_data.best_ub = 1e20;
	}

	if (_options.ALGORITHM == "ENHANCED_MULTICUT") {
		master_loop(stream);
	}
	else if (_options.ALGORITHM == "BASE" || _options.ALGORITHM == "IN-OUT") {
		while (!_data.stop) {
			classic_iteration(stream);
		}
	}else if(_options.ALGORITHM == "LEVEL"){
		solve_level(stream);
	}else {
		std::cout << "ERROR : UNKNOWN ALGORITHM " << std::endl;
		std::exit(0);
	}

	print_solution(stream, _data.x_cut, false, _data.global_prb_status, _options.PRINT_SOLUTION);

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
		else {
			_data.step_size = _options.STEP_SIZE;
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

	} while (_data.stay_in_x_cut);
	
	_data.ub /= float(_data.n_slaves_solved) / float(_data.nslaves);
	_data.ub += _data.invest_separation_cost;
}

int Benders::nbr_first_stage_vars()
{
	return _master->_id_to_name.size();
}

void Benders::solve_level(std::ostream& stream)
{
	// initialization
	_data.alpha_i.resize(_data.nslaves);
	_data.lb = _options.THETA_LB;

	int master_status = 0;

	_master->_solver->set_algorithm("DUAL");
	if (_options.INIT_MEAN_VALUE_SOLUTION) {
		_master->update_level_objective(_data.bestx);
	}

	while (!_data.stop) {

		// Compute level
		_data.level = _data.stab_value * _data.lb + (1 - _data.stab_value) * _data.best_ub;
		_master->update_level_constraint(_data.level);

		// 1. Solve master
		_data.timer_master.restart();
		//_master->_solver->set_output_log_level(3);
		_master->solve_quadratic(master_status);
		_data.time_master = _data.timer_master.elapsed();
		
		// 2. Check feasibility
		if (master_status != OPTIMAL) {
			// Case INFEASIBLE : LEVEL is too low
			_data.lb = _data.level;
		}
		else {
			// 3. get master solution
			_master->get(_data.x0, _data.alpha, _data.alpha_i); /*Get the optimal variables of the Master Problem*/
			compute_x_cut(_options, _data);

			// 4. compute and add cuts
			build_cut();

			// 5. update UB
			_data.invest_separation_cost = 0;
			int col_id = 0;
			for (auto const& kvp : _data.x_cut) {
				col_id = _master->_name_to_id[kvp.first];
				_data.invest_separation_cost += kvp.second * _master->_initial_obj[col_id];
			}
			_data.ub += _data.invest_separation_cost;

			// If the new solution has a cost deacreasing of at least 10% of the gap
			// we update the best solution and best UB
			double accept_param = 0.9;
			if (_data.ub < accept_param * _data.best_ub + (1-accept_param) * _data.level) {
				_data.best_ub = _data.ub;
				_data.bestx = _data.x_cut;
				_master->update_level_objective(_data.bestx);
			}
		}
		
		print_log(stream, _data, _options.LOG_LEVEL, _options);

		// 6. stopping criterion
		_data.stop = stopping_criterion(_data, _options);
		_data.it++;
	}



}

void Benders::solve_mean_value_problem(StrPairVector const& keys, DblVector const& values)
{
	for (int k(0); k < keys.size(); k++) {
		int id_col, id_row; 
		// 1. RHS
		if (keys[k].first == "RIGHT" ||
			keys[k].first == "RHS" ||
			keys[k].first == "RHS1") {
			id_row = _mean_value_prb->_solver->get_row_index(keys[k].second);
			_mean_value_prb->_solver->chg_rhs(id_row, values[k]);
		}
		// 2. MATRIX ELEMENT
		else {
			id_col = _mean_value_prb->_solver->get_row_index(keys[k].first);
			id_row = _mean_value_prb->_solver->get_row_index(keys[k].second);
			_mean_value_prb->_solver->chg_coef(id_row, id_col, values[k]);
		}
	}

	//_mean_value_prb->_solver->set_output_log_level(3);
	_mean_value_prb->_solver->set_algorithm("DUAL");
	int mean_status;
	_mean_value_prb->_solver->solve(mean_status, "");

	// Getting solution
	DblVector init_sol(_mean_value_prb->get_ncols(), 0.0);
	_mean_value_prb->get_MIP_sol(init_sol.data(), NULL);

	_x_init.clear();
	for (int i = 0; i < nbr_first_stage_vars(); i++) {
		_x_init[_master->_id_to_name[i]] = init_sol[i];
		//std::cout << _master->_id_to_name[i] << "   "  << init_sol[i] << "   " << _x_init[_master->_id_to_name[i]] << std::endl;
	}
}
