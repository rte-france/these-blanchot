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
Benders::Benders(CouplingMap const & problem_list, BendersOptions const & options) : _options(options) {

	if (!problem_list.empty()) {

		_data.nslaves = _options.SLAVE_NUMBER;
		if (_data.nslaves < 0) {
			_data.nslaves = problem_list.size() - 1;
		}

		auto it(problem_list.begin());
		
		auto const it_master = problem_list.find(_options.MASTER_NAME);
		Str2Int const & master_variable(it_master->second);
		for(int i(0); i < _data.nslaves; ++it) {
			if (it != it_master) {
				_problem_to_id[it->first] = i;
				_map_slaves[it->first] = WorkerSlavePtr(new WorkerSlave(it->second, _options.get_slave_path(it->first), _options.slave_weight(_data.nslaves, it->first), _options));
				_slaves.push_back(it->first);
				i++;
			}
		}

		std::cout << it_master->first << " " << _options.get_master_path() << std::endl;
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

	double best_alpha = 0;
	double best_lb = -1e20;
	double cur_lb = 0;
	double cur_ub;
	double last_lb = _data.lb;
	double cur_gap;
	double best_gap = 1e20;
	double best_ub = 1e20;
	double last_ub = 1e20;

	/***************************************
	****************************************
			   ON UPPER BOUND
	****************************************
	***************************************/
	if (_options.ALPHA_STRAT == "UB") {

		best_alpha = 0;
		best_lb = -1e20;
		cur_lb = 0;
		cur_ub;
		last_lb = _data.lb;
		cur_gap;
		best_gap = 1e20;
		best_ub = 1e20;
		last_ub = 1e20;
		
		int partition = 200;

		bool mini = false;
		double alphamin = 0.0;
		double alphamax = 1.0;
		int status;
		double grad;
		double val;
		int iterations;

		while (!mini) {
			//_data.stab_value = (1.0 / partition) * i;
			compute_x_cut(_options, _data);
			cur_lb = 0;
			_data.ub = 0;
			cur_ub = 0;
			grad = 0;

			// 1. on evalue 0.01 au dessus
			_data.stab_value += 0.05;
			compute_x_cut(_options, _data);
			for (auto& kvp : _map_slaves) {
				WorkerSlavePtr& ptr(kvp.second);
				ptr->fix_to(_data.x_cut);
				ptr->solve(status, _options, _data.nslaves, kvp.first);
				ptr->get_value(val);
				grad += val;
			}
			compute_separation_point_cost(_master, _data, _options);
			grad += _data.invest_separation_cost;
			
			// 2. on evalue en le point
			_data.stab_value -= 0.05;
			compute_x_cut(_options, _data);
			for (auto& kvp : _map_slaves) {
				WorkerSlavePtr& ptr(kvp.second);
				ptr->fix_to(_data.x_cut);
				ptr->set_simplex_iter(1000);
				ptr->solve(status, _options, _data.nslaves, kvp.first);
				ptr->get_value(val);
				ptr->set_simplex_iter(50000);
				cur_ub += val;
				ptr->get_simplex_ite(iterations);
			}
			compute_separation_point_cost(_master, _data, _options);
			cur_ub += _data.invest_separation_cost;

			// 3. on evalue le gradient
			grad -= cur_ub;

			if (grad > 0) {
				alphamax = _data.stab_value;
			}
			else {
				alphamin = _data.stab_value;
			}

			/*std::cout << std::fixed << std::setprecision(3) << std::setw(10) << _data.stab_value
				<< std::scientific << std::setprecision(8) << std::setw(30) << cur_ub
				<< std::scientific << std::setprecision(8) << std::setw(30) << grad
				<< std::scientific << std::setprecision(8) << std::setw(30) << alphamin
				<< std::scientific << std::setprecision(8) << std::setw(30) << alphamax
				<< std::endl;*/

			/*if (cur_ub < best_ub) {
				best_ub = cur_ub;
				best_alpha = _data.stab_value;
			}*/

			best_alpha = _data.stab_value;

			if (alphamax - alphamin > 1e-2) {
				_data.stab_value = alphamin + (alphamax - alphamin) / 2;
			}
			else {
				mini = true;

			}

			if ( abs(grad) < 1e-3 ) {
				mini = true;
			}

		}
	}
	/***************************************
	****************************************
	           ON LOWER BOUND
	****************************************
	***************************************/
	else if (_options.ALPHA_STRAT == "LB") {
		best_alpha = 0;
		best_lb = -1e20;
		cur_lb = 0;
		cur_ub;
		last_lb = _data.lb;
		cur_gap;
		best_gap = 1e20;
		best_ub = 1e20;
		last_ub = 1e20;

		int partition = 100;

		bool mini = false;
		double alphamin = 0.0;
		double alphamax = 1.0;
		int status;
		double grad;
		double val;
		int iterations;

		std::string name = "bounds.txt";
		if (_data.it == 1) {
			std::ofstream sortie(name.c_str());
		}
		else {
			std::ofstream sortie(name.c_str(), std::ios::app);
		}
		std::ofstream sortie(name.c_str(), std::ios::app);

		sortie << std::setw(10) << _data.it;

		for (int i(1); i <= partition; i++) {
			// choix de alpha
			_data.stab_value = float(i) / partition;
			compute_x_cut(_options, _data);

			// construire les coupes
			build_cut();

			// plein de trucs relatifs a tout le blabla
			reset_iteration_data(_data, _options);
			get_master_value(_master, _data, _options);
			compute_ub(_master, _data);
			cur_gap = _data.ub - _data.lb;

			if (cur_gap <= best_gap) {
				best_gap = cur_gap;
				best_alpha = _data.stab_value;
			}

			// on ecrit le resultat dans un fichier pour faire des courbes parce que les courbes cest top
			sortie << std::scientific << std::setprecision(8) << std::setw(30) << _data.lb << "_" << _data.ub;
			/*std::cout	<< std::scientific << std::setprecision(2) << std::setw(10) << _data.stab_value 
						<< std::scientific << std::setprecision(8) << std::setw(25) << cur_gap << std::endl;
		
			*/
			// on supprime les coupes et on reprend l'ancienne solution du master
			del_last_rows(_master, _options, _data);
			reset_iteration_data(_data, _options);
			get_master_value(_master, _data, _options);
		}
		sortie << std::endl;
		//std::cout << std::endl;
		sortie.close();
	}

	if (_options.ALPHA_STRAT == "UB" || _options.ALPHA_STRAT == "LB") {
		_data.stab_value = best_alpha;
	}
	
	_data.ub = 0;

	compute_x_cut(_options, _data);
	build_cut();
	compute_ub(_master, _data);
	
	update_in_out_stabilisation(_master, _data);
	if (_options.ALPHA_STRAT != "ALL") {
		_data.stab_value = best_alpha;
	}

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

	while (!_data.stop) {

		//_data.step_size = 0.1;

		// 1. resolution of master problem
		get_master_value(_master, _data, _options);
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
		if (_data.misprices == 0) {
			//_data.step_size = std::max(0.1, 0.8 * _data.step_size);
		}

		if (_data.stop) {
			print_log(stream, _data, _options.LOG_LEVEL, _options);
		}

	}
}

void Benders::separation_loop(std::ostream& stream)
{
	while ( _data.has_cut == false ) {

		set_slaves_order(_data, _options);

		// 1. Compute separation point
		compute_x_cut(_options, _data);

		// 2. Compute difference of first stage solutions objectives
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
}
