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
Benders::Benders(CouplingMap const & problem_list, BendersOptions const & options, AbstractSolver* solver) : _options(options) {
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
				if(options.SOLVER == ""){
					std::cout << "SOLVEUR NON RECONNU" << std::endl;
					std::exit(1);
				}
				#ifdef XPRESS
					else if(options.SOLVER == "XPRESS"){
						_map_slaves[it->first] = WorkerSlavePtr(new WorkerSlaveXPRS(it->second, _options.get_slave_path(it->first), _options.slave_weight(_data.nslaves, it->first), _options));
					}
				#endif
				#ifdef CPLEX
					else if(options.SOLVER == "CPLEX"){
						_map_slaves[it->first] = WorkerSlavePtr(new WorkerSlaveCPLX(it->second, _options.get_slave_path(it->first), _options.slave_weight(_data.nslaves, it->first), _options, solver));
					}
				#endif
				else{
					std::cout << "SOLVEUR NON RECONNU" << std::endl;
					std::exit(1);
				}
				_slaves.push_back(it->first);
				i++;
			}
		}

		std::cout << it_master->first << " " << _options.get_master_path() << std::endl;
		if(options.SOLVER == ""){
			std::cout << "SOLVEUR NON RECONNU" << std::endl;
			std::exit(1);
		}
		#ifdef XPRESS
			else if(options.SOLVER == "XPRESS"){
				_master.reset(new WorkerMasterXPRS(master_variable, _options.get_master_path(), _options, _data.nslaves));
			}
		#endif
		#ifdef CPLEX
			else if(options.SOLVER == "CPLEX"){
				_master.reset(new WorkerMasterCPLX(master_variable, _options.get_master_path(), _options, solver, _data.nslaves));
		}
		#endif
		else{
			std::cout << "SOLVEUR NON RECONNU" << std::endl;
			std::exit(1);
		}
	}

	_data.eta = _options.ETA_IN_OUT;

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

	if (_options.RAND_AGGREGATION) {
		// if(_data.it <= _data.nslaves){
		// 	_options.SAMPLING_STRATEGY == "ORDERED";
		// }else{
		// 	_options.SAMPLING_STRATEGY == "MAX_GAP";
		// }

		if(_data.it < 2){
			get_slave_cut(slave_cut_package, _map_slaves, _options, _data, _problem_to_id);
			for(int i=0; i<_data.nslaves; i++){
				_data.gap_i[i] = _data.min_val_i[i] - _data.alpha_i[i];
			}
			/*
			if( _data.solve_master ){
				std::rotate(_data.indices.begin(), _data.indices.begin()+_data.nbr_sp_no_cut+1, _data.indices.end());
			}
			// New resolution
			_data.has_cut_this_ite = false;
			// Solving nrandom SP only if there are still at least nrandom SP to solve, if not, solving the left SP
			_data.nbr_sp_to_solve = std::min(_data.nrandom, _data.nslaves - _data.nbr_sp_no_cut);
			get_random_slave_cut(slave_cut_package, _map_slaves, _slaves, _options, _data, _problem_to_id);

			if( _data.has_cut_this_ite ){
				_data.solve_master = true;
			}else{
				_data.solve_master = false;
			}*/

		}else{
			// Si solve_master == True, on est dans une iteration avec de nouvelles variables de premier niveau
			// On shuffle les sous-problemes
			// Sinon, on continue a tirer des sous-problemes associes a un shuffle jusqu'a couper
			if( _data.solve_master ){
				if(_options.SAMPLING_STRATEGY == "RANDOM"){
					//std::random_shuffle(_slaves.begin(), _slaves.end());
					std::random_shuffle(_data.indices.begin(), _data.indices.end());
					
				}else if(_options.SAMPLING_STRATEGY == "ORDERED"){
					//std::rotate(_slaves.begin(), _slaves.begin()+_data.nbr_sp_no_cut+1,_slaves.end());
					std::rotate(_data.indices.begin(), _data.indices.begin()+_data.nbr_sp_no_cut+1, _data.indices.end());
				}else if(_options.SAMPLING_STRATEGY == "PSEUDOCOST"){
					if(*std::min_element(_data.pseudocost.begin(), _data.pseudocost.end()) < 1e-6){
						std::rotate(_data.indices.begin(), _data.indices.begin()+_data.nbr_sp_no_cut+1, _data.indices.end());
						std::cout << "MIN " << *std::min_element(_data.pseudocost.begin(), _data.pseudocost.end()) << std::endl;
						std::cout << "MAX " << *std::max_element(_data.pseudocost.begin(), _data.pseudocost.end()) << std::endl;
					}else{
						//std::exit(1);
						std::sort(_data.indices.begin(), _data.indices.end(), [&](const int i, const int j){return _data.pseudocost[i] > _data.pseudocost[j]; });
						std::cout << "MIN " << *std::min_element(_data.pseudocost.begin(), _data.pseudocost.end()) << std::endl;
						std::cout << "MAX " << *std::max_element(_data.pseudocost.begin(), _data.pseudocost.end()) << std::endl;
						std::cout << "CHOIX " << _slaves[_data.indices[0]] << "  " << _data.pseudocost[_data.indices[0]] << std::endl;
						//sort_slaves_by_pseudocost();	
					}
				}else if(_options.SAMPLING_STRATEGY == "ORDERED_RANDOMIZED"){
					//std::rotate(_slaves.begin(), _slaves.begin()+_data.nbr_sp_no_cut+1,_slaves.end());
					if(_data.it == 2){
						std::random_shuffle(_data.indices.begin(), _data.indices.end());
					}
					std::rotate(_data.indices.begin(), _data.indices.begin()+_data.nbr_sp_no_cut, _data.indices.end());
				}else if(_options.SAMPLING_STRATEGY == "RANDOM_EPOCH"){
					if(_data.to_shuffle){
						std::random_shuffle(_data.indices.begin(), _data.indices.end());
						_data.first_id = _data.indices[0];
						_data.to_shuffle = false;
					}
					std::rotate(_data.indices.begin(), _data.indices.begin()+_data.nbr_sp_no_cut+1, _data.indices.end());
					for(int i=0; i < _data.nrandom; i++){
						if(_data.indices[i] == _data.first_id){
							_data.to_shuffle = true;
						}
					}
				}else if(_options.SAMPLING_STRATEGY == "MAX_GAP"){
					// 1. calcul des gap
					if(_data.has_cut_this_ite){
						for(int i=0; i<_data.nslaves; i++){
							_data.gap_i[i] = _data.min_val_i[i] - _data.alpha_i[i];
						}
						// 2. tri

						std::sort(_data.indices.begin(), _data.indices.end(), [&](const int i, const int j){ return _data.gap_i[i] > _data.gap_i[j]; });	
						//std::cout << _data.gap_i[_data.indices[0]] << std::endl;
						_data.current_gap = _data.gap_i[_data.indices[0]];
						if(_data.gap_i[_data.indices[0]] < (_options.GAP / _data.nslaves) ){
							_options.ALGORITHM = "INOUT";
							_data.nrandom = _data.nslaves;
							_options.RAND_AGGREGATION = 0;
							std::cout << "SWITCH STRATEGY" << std::endl;
						}
					}
				}
				_data.nbr_sp_no_cut = 0;
			}
			// New resolution
			_data.has_cut_this_ite = false;
			// Solving nrandom SP only if there are still at least nrandom SP to solve, if not, solving the left SP
			_data.nbr_sp_to_solve = std::min(_data.nrandom, _data.nslaves - _data.nbr_sp_no_cut);
			get_random_slave_cut(slave_cut_package, _map_slaves, _slaves, _options, _data, _problem_to_id);

			if( _data.has_cut_this_ite ){
				_data.solve_master = true;
			}else{
				_data.solve_master = false;
			}
		}
	}
	else {
		get_slave_cut(slave_cut_package, _map_slaves, _options, _data, _problem_to_id);

	}
	_data.timer_slaves = timer_slaves.elapsed();
	all_package.push_back(slave_cut_package);
	build_cut_full(_master, all_package, _problem_to_id, _trace, _slave_cut_id, _all_cuts_storage, _dynamic_aggregate_cuts, _data, _options);
	if (_options.BASIS) {
		SimplexBasisPackage slave_basis_package;
		AllBasisPackage all_basis_package;
		get_slave_basis(slave_basis_package, _map_slaves);
		all_basis_package.push_back(slave_basis_package);
		sort_basis(all_basis_package, _problem_to_id, _basis, _data);

	}
}

/*!
*  \brief Run Benders algorithm
*
*  Method to run Benders algorithm
*
* \param stream : stream to print the output
*/
void Benders::run(std::ostream & stream, AbstractSolver* solver) {
	
	init_log(stream, _options.LOG_LEVEL, _options);
	for (auto const & kvp : _problem_to_id) {
		_all_cuts_storage[kvp.first] = SlaveCutStorage();
	}
	init(_data, _options);
	_data.nrandom = _options.RAND_AGGREGATION;

	while (!_data.stop) {
		if(_options.ALGORITHM == "INOUT" || _options.ALGORITHM == "BASE"){
			perform_one_inout_iteration(stream);
		}else if(_options.ALGORITHM == "SAMPLING"){
			perform_one_sampling_iteration(stream);
		}
	}

	if(_options.ALGORITHM == "SAMPLING"){
		_data.bestx = _data.x0;
	}

	print_solution(stream, _data.bestx, true);
	if (_options.TRACE) {
		print_csv(_trace, _problem_to_id, _data, _options);
	}
	if (_options.ACTIVECUTS) {
		print_active_cut(_active_cuts,_options);
	}
}

/*!
*  \brief Perform one iteration of InOut Algorithm
*
*  Method to Perform one iteration of InOut Algorithm
*
* \param stream : stream to print the output
*/
void Benders::perform_one_inout_iteration(std::ostream & stream) {
	Timer timer_master;
	++_data.it;
	get_master_value(_master, _data, _options);

	// on recupere les bornes initiales sur les variables d'investissement
	if(_data.it == 1){
		save_bounds(_master, _data, _options);
	}

	// Calcul du point de coupe xcut et on evalue l'obj en ce nouveau point
	if(_options.ALGORITHM == "INOUT"){
		compute_x_cut(_master, _data, _options);
	}

	if (_options.ACTIVECUTS) {
		update_active_cuts(_master, _active_cuts, _slave_cut_id, _data.it);
	}

	if (_options.TRACE) {
		_trace.push_back(WorkerMasterDataPtr(new WorkerMasterData));
	}
	build_cut();

	update_best_ub(_data.best_ub, _data.ub, _data.bestx, _data.x0, _data.eta, _options.DYNAMIC_STABILIZATION);

	if (_options.TRACE) {
		update_trace(_trace, _data);
	}
	_data.timer_master = timer_master.elapsed();
	print_log(stream, _data, _options.LOG_LEVEL, _options);
	_data.stop = stopping_criterion(_data, _options);
}

/*!
*  \brief Perform one iteration of InOut Algorithm
*
*  Method to Perform one iteration of InOut Algorithm
*
* \param stream : stream to print the output
*/
void Benders::perform_one_sampling_iteration(std::ostream & stream) {
	Timer timer_master;
	++_data.it;

	if(_data.solve_master){
		// On conserve le point precedent avant de resoudre a nouveau le master
		_data.previous_x 	= _data.x0;
		_data.previous_lb 	= _data.lb;
		std::cout << "COUOUC" << std::endl;
		
		// On remet le gap restant a 0
		_data.remaining_gap = _options.GAP;

		_data.timer_master = timer_master.elapsed();
		get_master_value(_master, _data, _options);
		_data.timer_master = timer_master.elapsed();
		compute_delta_x(_master, _data, _options);
		compute_pseudocosts(_data, _options);
	}
	
	compute_x_momentum(_master, _data, _options);

	// if (_options.ACTIVECUTS) {
	// 	update_active_cuts(_master, _active_cuts, _slave_cut_id, _data.it);
	// }

	// if (_options.TRACE) {
	// 	_trace.push_back(WorkerMasterDataPtr(new WorkerMasterData));
	// }

	build_cut();
	//update_best_ub(_data.best_ub, _data.ub, _data.bestx, _data.x0, _data.eta, _options.DYNAMIC_STABILIZATION);

	// if (_options.TRACE) {
	// 	update_trace(_trace, _data);
	// }
	
	if(_data.it % 1 == 0){
		print_log(stream, _data, _options.LOG_LEVEL, _options);
	}
	_data.stop = stopping_criterion(_data, _options);

}