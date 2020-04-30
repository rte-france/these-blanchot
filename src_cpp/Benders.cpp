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

	int partition = 10;
	int rows;

	for (int i(1); i <= partition; i++) {
		_data.stab_value = (1.0/partition) * i;
		compute_x_cut(_options, _data);
		cur_lb = 0;
		
		/*for (auto& kvp : _map_slaves) {
			WorkerSlavePtr& ptr(kvp.second);
			SlaveCutDataPtr slave_cut_data(new SlaveCutData);
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			ptr->fix_to(_data.x_cut);
			ptr->solve(handler->get_int(LPSTATUS), _options, _data.nslaves, kvp.first);
			ptr->get_value(handler->get_dbl(SLAVE_COST));

			cur_lb += (handler->get_dbl(SLAVE_COST));
		}
		compute_separation_point_cost(_master, _data, _options);
		cur_lb += _data.invest_separation_cost;*/

		//std::cout << "BEFORE " << std::setw(10) << _master->get_nrows() << "   " << std::setprecision(8) << _data.lb << std::endl;

		build_cut();
		// 1. resoudre master
		get_master_value(_master, _data, _options);
		//std::cout << "DURING " << std::setw(10) << _master->get_nrows() << "   " << std::setprecision(8) << _data.lb << std::endl;
		cur_lb = _data.lb;
		std::cout	<< std::fixed		<< std::setprecision(2) << std::setw(10) << _data.stab_value 
					<< std::scientific	<< std::setprecision(8) << std::setw(30) << cur_lb << std::endl;
		// 2. supprimer rows
		del_last_rows(_master, _options, _data);

		// 3. resoudre master !
		get_master_value(_master, _data, _options);
		//std::cout << "AFTER  " << std::setw(10) << _master->get_nrows() << "   " << std::setprecision(8) << _data.lb << std::endl << std::endl;

		if (cur_lb > best_lb) {
			best_lb = cur_lb;
			best_alpha = _data.stab_value;
		}
	}

	_data.stab_value = best_alpha;
	if (best_lb == _data.lb) {
		_data.stab_value = 1.0;
	}
	
	compute_x_cut(_options, _data);
	build_cut();
	compute_ub(_master, _data);
	
	//update_in_out_stabilisation(_master, _data);
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

		// 1. resolution of master problem
		get_master_value(_master, _data, _options);
		_data.has_cut = false;
		
		// 2. Choosing new order of subpoblems
		set_slaves_order(_data, _options);

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
			//_data.step_size = std::min(1.0, _data.step_size * (1.0 + (1.0 / _data.misprices)));
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
