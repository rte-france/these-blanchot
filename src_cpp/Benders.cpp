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
Benders::Benders(CouplingMap const & problem_list, BendersOptions const & options) {
	_options = options;
	if (!problem_list.empty()) {
		_data.nslaves = _options.SLAVE_NUMBER;
		if (_data.nslaves < 0) {
			_data.nslaves = problem_list.size() - 1;
		}
		bool stop = false;
		auto it(problem_list.begin());
		auto end(problem_list.end());
		
		int i(0);
		auto it_master = problem_list.find(_options.MASTER_NAME);
		std::string const & master_name(it_master->first);
		std::map<std::string, int> const & master_variable(it_master->second);
		for(int i(0); i < _data.nslaves; ++it) {
			if (it != it_master) {
				_problem_to_id[it->first] = i;
				_slaves[it->first] = WorkerSlavePtr(new WorkerSlave(it->second, _options.get_slave_path(it->first)));
				i++;
			}
		}
		init_slave_weight(_data, _options, _slave_weight_coeff, _problem_to_id);
		std::cout << it_master->first << " " << _options.get_master_path() << std::endl;
		_master.reset(new WorkerMaster(master_variable, _options.get_master_path(), _slave_weight_coeff, _data.nslaves));
	}

}


/*!
*  \brief Method to free the memory used by each problem
*/
void Benders::free() {
	_master->free();
	for (auto & ptr : _slaves)
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
	std::vector<SlaveCutPackage> all_package;
	Timer timer_slaves;
	if (_options.RAND_CUTS) {
		select_random_slaves(_problem_to_id, _options, _random_slaves);
		get_random_slave_cut(slave_cut_package, _slaves, _random_slaves, _options, _data);
	}
	else {
		get_slave_cut(slave_cut_package, _slaves, _options, _data);
	}
	_data.timer_slaves = timer_slaves.elapsed();
	all_package.push_back(slave_cut_package);
	build_cut_full(_master, _slave_weight_coeff, all_package, _problem_to_id, _random_slaves, _trace, _slave_cut_id, _all_cuts_storage, _dynamic_aggregate_cuts, _data, _options);
	if (_options.BASIS) {
		SimplexBasisPackage slave_basis_package;
		std::vector<SimplexBasisPackage> all_basis_package;
		get_slave_basis(slave_basis_package, _slaves);
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
void Benders::run(std::ostream & stream) {

	WorkerMaster & master(*_master);
	
	init_log(stream, _options.LOG_LEVEL);
	for (auto const & kvp : _problem_to_id) {
		_all_cuts_storage[kvp.first] = SlaveCutStorage();
	}
	init(_data);
	while (!_data.stop) {
		Timer timer_master;
		++_data.it;
		get_master_value(_master, _data, _options);
		if (_options.ACTIVECUTS) {
			update_active_cuts(_master, _active_cuts, _slave_cut_id, _data.it);
		}

		if (_options.TRACE) {
			_trace.push_back(WorkerMasterDataPtr(new WorkerMasterData));
		}
		build_cut();

		update_best_ub(_data.best_ub, _data.ub, _data.bestx, _data.x0);

		if (_options.TRACE) {
			update_trace(_trace, _data);
		}
		_data.timer_master = timer_master.elapsed();
		print_log(stream, _data, _options.LOG_LEVEL);
		_data.stop = stopping_criterion(_data, _options);
	}
	
	print_solution(stream, _data.bestx, true);
	if (_options.TRACE) {
		print_csv(_trace, _problem_to_id, _data, _options);
	}
	if (_options.ACTIVECUTS) {
		print_active_cut(_active_cuts,_options);
	}
}