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
	get_slave_cut(slave_cut_package, _slaves, _options, _data);
	all_package.push_back(slave_cut_package);
	if (!_options.AGGREGATION) {
		sort_cut_slave(all_package, _slave_weight_coeff, _master, _problem_to_id, _trace, _all_cuts_storage, _data, _options);
	}
	else {
		sort_cut_slave_aggregate(all_package, _slave_weight_coeff, _master, _problem_to_id, _trace, _all_cuts_storage, _data, _options);
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
	init(_data);
	for (auto const & kvp : _problem_to_id) {
		_all_cuts_storage[kvp.first] = SlaveCutStorage();
	}
	master.write(0);
	while (!_data.stop) {

		++_data.it;
		get_master_value(_master, _data);
		if (_options.TRACE) {
			_trace._master_trace.push_back(WorkerMasterDataPtr(new WorkerMasterData));
		}
		build_cut();

		update_best_ub(_data.best_ub, _data.ub, _data.bestx, _data.x0);

		//Update best upper bound 
		print_log(stream, _data, _options.LOG_LEVEL);

		if (_options.TRACE) {
			update_trace(_trace, _data);
		}

		_data.stop = stopping_criterion(_data, _options);
	}
	
	print_solution(stream, _data.bestx, true);
	//if (_options.TRACE) {
	//	print_csv();
	//}

}


/*!
*  \brief Print the trace of the Benders algorithm in a csv file
*
*  Method to print trace of the Benders algorithm in a csv file
*
* \param stream : stream to print the output
*/
//void Benders::print_csv() {
//	std::string output(_options.ROOTPATH + PATH_SEPARATOR + "benders_output.csv");
//	if (_options.AGGREGATION) {
//		output = (_options.ROOTPATH + PATH_SEPARATOR + "benders_output_aggregate.csv");
//	}
//	std::ofstream file(output, std::ios::out | std::ios::trunc);
//
//	if (file)
//	{
//		file << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;deletedcut" << std::endl;
//		std::size_t found = _options.MASTER_NAME.find_last_of(PATH_SEPARATOR);
//		Point xopt;
//		int nite;
//		nite = _trace.get_ite();
//		xopt = _trace._master_trace[nite-1]->get_point();
//		for (int i(0); i < nite; i++) {
//			file << i + 1 << ";";
//			file << "Master" << ";";
//			file<< _options.MASTER_NAME.substr(found+1) << ";";
//			file << _slaves.size() << ";";
//			file << _trace._master_trace[i]->get_ub() << ";";
//			file << _trace._master_trace[i]->get_lb() << ";";
//			file << _trace._master_trace[i]->get_bestub() << ";";
//			file << norm_point(xopt, _trace._master_trace[i]->get_point()) << ";";
//			file << _trace._master_trace[i]->get_deletedcut() << std::endl;
//			for (auto & kvp : _trace._master_trace[i]->_cut_trace) {
//				SlaveCutDataHandler handler(kvp.second);
//				file << i + 1 << ";";
//				file << "Slave" << ";";
//				file << kvp.first.substr(found+1) << ";";
//				file << _problem_to_id[kvp.first] << ";";
//				file << handler.get_dbl(SLAVE_COST) << ";";
//				file << handler.get_dbl(ALPHA_I) << ";";
//				file << ";";
//				file << handler.get_int(SIMPLEXITER) << ";";
//				file << std::endl;
//			}
//		}
//		file.close();
//	}
//	else {
//		std::cout << "Impossible d'ouvrir le fichier .csv" << std::endl;
//	}
//}