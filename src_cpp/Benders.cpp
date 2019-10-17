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
				if(options.SOLVER == "XPRESS"){
					_map_slaves[it->first] = WorkerSlavePtr(new WorkerSlaveXPRS(it->second, _options.get_slave_path(it->first), _options.slave_weight(_data.nslaves, it->first), _options));
				}else if(options.SOLVER == "CPLEX"){
					_map_slaves[it->first] = WorkerSlavePtr(new WorkerSlaveCPLX(it->second, _options.get_slave_path(it->first), _options.slave_weight(_data.nslaves, it->first), _options, solver));
				}else{
					std::cout << "SOLVEUR NON RECONNU" << std::endl;
					std::exit(1);
				}
				_slaves.push_back(it->first);
				i++;
			}
		}

		std::cout << it_master->first << " " << _options.get_master_path() << std::endl;
		if(options.SOLVER == "XPRESS"){
			_master.reset(new WorkerMasterXPRS(master_variable, _options.get_master_path(), _options, _data.nslaves));
		}else if(options.SOLVER == "CPLEX"){
			_master.reset(new WorkerMasterCPLX(master_variable, _options.get_master_path(), _options, solver, _data.nslaves));
		}else{
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
		std::random_shuffle(_slaves.begin(), _slaves.end());
		get_random_slave_cut(slave_cut_package, _map_slaves, _slaves, _options, _data);
	}
	else {
		get_slave_cut(slave_cut_package, _map_slaves, _options, _data);
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
	
	init_log(stream, _options.LOG_LEVEL);
	for (auto const & kvp : _problem_to_id) {
		_all_cuts_storage[kvp.first] = SlaveCutStorage();
	}
	init(_data);
	_data.nrandom = _options.RAND_AGGREGATION;

	while (!_data.stop) {
		Timer timer_master;
		++_data.it;
		get_master_value(_master, _data, _options);

		// on recupere les bornes initiales sur les variables d'investissement
		if(_data.it == 1){
			save_bounds(_master, _data, _options);
		}

		// Calcul du point de coupe xcut et on evalue l'obj en ce nouveau point
		compute_x_cut(_master, _data, _options);

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
		print_log(stream, _data, _options.LOG_LEVEL);
		_data.stop = stopping_criterion(_data, _options);
	}
	
	// on ecrit les iterations dans un fichier
	std::ofstream fichier("Inout_iterations.txt", std::ios::out | std::ios::app);
	if(fichier){
		fichier << _options.ETA_IN_OUT << "     " << _data.it << std::endl;
		fichier.close();
	}

	print_solution(stream, _data.bestx, true);
	if (_options.TRACE) {
		print_csv(_trace, _problem_to_id, _data, _options);
	}
	if (_options.ACTIVECUTS) {
		print_active_cut(_active_cuts,_options);
	}
}