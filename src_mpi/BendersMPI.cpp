#include "BendersMPI.h"



BendersMpi::~BendersMpi() {

}

BendersMpi::BendersMpi() {

}

/*!
*  \brief Get the slave weight from a input file
*
*  Method to build the weight coefficients vector from an input file stored in the options
*
*  \param problemroot : root where are stored the problem
*/
void BendersMpi::init_slave_weight(std::string problemroot) {
	_slave_weight_coeff.resize(_data.nslaves);
	if (_options.SLAVE_WEIGHT == "UNIFORM") {
		for (int i(0); i < _data.nslaves; i++) {
			_slave_weight_coeff[i] = 1 / static_cast<double>(_data.nslaves);
		}
	}
	else if (_options.SLAVE_WEIGHT == "ONES") {
		for (int i(0); i < _data.nslaves; i++) {
			_slave_weight_coeff[i] = 1;
		}
	}
	else {
		std::string line;
		std::size_t found = problemroot.find_last_of(PATH_SEPARATOR);
		std::string root;
		root = problemroot.substr(0, found);
		std::string filename = root + PATH_SEPARATOR + _options.SLAVE_WEIGHT;
		std::ifstream file(filename);
		if (!file) {
			std::cout << "Cannot open file " << filename << std::endl;
		}
		while (std::getline(file, line))
		{
			std::stringstream buffer(line);
			std::string problem_name;
			buffer >> problem_name;
			problem_name = root + PATH_SEPARATOR + problem_name;
			buffer >> _slave_weight_coeff[_problem_to_id[problem_name]];
			std::cout << problem_name << " : " << _problem_to_id[problem_name] << "  :  " <<  _slave_weight_coeff[_problem_to_id[problem_name]] << std::endl;
		}
	}
}

/*!
*  \brief Method to load each problem in a thread
*
*  The initialization of each problem is done sequentially 
*
*  \param problem_list : list of string containing problems' names
*/

void BendersMpi::load(CouplingMap const & problem_list, mpi::environment & env, mpi::communicator & world, BendersOptions const & options) {

	if (!problem_list.empty()) {

		std::vector<mps_order> send_orders;

		_data.nslaves = static_cast<int>(problem_list.size()) - 1;
		auto it(problem_list.begin());
		auto end(problem_list.end());
		_options = options;
		int rank(1);
		if (world.rank() == 0) {

			send_orders.reserve(problem_list.size() - 1);

			auto it_master = problem_list.find(_options.MASTER_NAME);
			std::string master_name(it_master->first);
			std::map<std::string, int> master_variable(problem_list.find(_options.MASTER_NAME)->second);
			++it;
			int i(0);
			while (it != end) {
				if (it == it_master) {
					++it;
				}
				else {
					if (rank >= world.size()) {
						rank = 1;
					}
					send_orders.push_back({ rank, it->first });
					_problem_to_id[it->first] = i;
					++rank;
					++it;
					i++;
				}
			}
			init_slave_weight(it_master->first);

			_master.reset(new WorkerMaster(master_variable, master_name, _slave_weight_coeff, _data.nslaves));

		}

		bool stop_communication(false);
		std::string mps;
		std::vector<mps_order>::const_iterator it_order(send_orders.begin());
		std::vector<mps_order>::const_iterator end_order(send_orders.end());

		while (!stop_communication) {
			if (world.rank() == 0) {
				if (it_order == end_order) {
					stop_communication = true;
				}
				else {
					mps = it_order->second;
					rank = it_order->first;
					++it_order;
				}
			}
			mpi::broadcast(world, stop_communication, 0);
			world.barrier();
			if (!stop_communication) {

				mpi::broadcast(world, mps, 0);
				mpi::broadcast(world, rank, 0);

				if (world.rank() != 0) {
					if (world.rank() == rank) {
						_map_slaves[mps] = WorkerSlavePtr(new WorkerSlave(problem_list.find(mps)->second, mps));
					}
				}
				world.barrier();
			}
		}
	}
}


/*!
*  \brief Solve, get and send solution of the Master Problem to every thread
*/

void BendersMpi::step_1(mpi::environment & env, mpi::communicator & world) {

	if (world.rank() == 0)
	{
		_data.deletedcut = 0;
		_data.maxsimplexiter = 0;
		_data.minsimplexiter = 1000000;
		_data.alpha_i.resize(_data.nslaves);
		_data.alpha = 0;

		_master->solve();
		_master->get(_data.x0, _data.alpha, _data.alpha_i);
		_master->get_value(_data.lb);

		if (_options.TRACE) {
			_trace._master_trace.push_back(WorkerMasterDataPtr(new WorkerMasterData));
		}

		_data.invest_cost = _data.lb - _data.alpha;
		_data.ub = _data.invest_cost;

	}


	broadcast(world, _data.x0, 0);

	world.barrier();

}

/*!
*  \brief Get cut information of every Slave Problem in each thread and send it to thread 0
*
*/
void BendersMpi::step_2(mpi::environment & env, mpi::communicator & world) {
	if (world.rank() != 0)
	{
		SlaveCutPackage slave_cut_package;

		for (auto & kvp : _map_slaves) {
			IntVector intParam(SlaveCutInt::MAXINT);
			DblVector dblParam(SlaveCutDbl::MAXDBL);
			SlaveCutDataPtr slave_cut_data(new SlaveCutData);
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			WorkerSlavePtr & ptr(kvp.second);

			ptr->fix_to(_data.x0);
			ptr->solve();
			ptr->get_basis();

			ptr->get_value(handler->get_dbl(SLAVE_COST));
			ptr->get_subgradient(handler->get_subgradient());
			ptr->get_simplex_ite(handler->get_int(SIMPLEXITER));

			SlaveCutTrimmer trimmercut(handler, _data.x0);
			slave_cut_package[kvp.first] = *slave_cut_data;
	
		}
		gather(world, slave_cut_package, 0);
	}
	world.barrier();
}

/*!
*  \brief Add cut to Master Problem and store the cut in a set
*
*  Method to add cut from a slave to the Master Problem and store this cut in a map linking each slave to its set of cuts.
*
*  \param slave_cut_package : cut information
*/
void BendersMpi::sort_cut_slave(SlaveCutPackage & slave_cut_package) {
	for (auto & itmap : slave_cut_package) {

		SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

		handler->get_subgradient() = itmap.second.first.first.first;
		handler->get_dbl(ALPHA_I) = _data.alpha_i[_problem_to_id[itmap.first]];

		_data.ub += handler->get_dbl(SLAVE_COST)* _slave_weight_coeff[_problem_to_id[itmap.first]];

		SlaveCutTrimmer cut(handler, _data.x0);
		if (!(_all_cuts_storage[itmap.first].find(cut) == _all_cuts_storage[itmap.first].end())) {
			_data.deletedcut++;
		}
		else {
			_master->add_cut_slave(_problem_to_id[itmap.first], handler->get_subgradient(), _data.x0, handler->get_dbl(SLAVE_COST));
			_all_cuts_storage[itmap.first].insert(cut);
		}

		if (_options.TRACE) {
			_trace._master_trace[_data.it - 1]->_cut_trace[itmap.first] = slave_cut_data;
		}

		bound_simplex_iter(handler->get_int(SIMPLEXITER));
	}
}

/*!
*  \brief Method to gather every cut information from each thread in thread 0 and add cut to Master problem
*
*/
void BendersMpi::step_3(mpi::environment & env, mpi::communicator & world) {
	if (world.rank() == 0) {

		SlaveCutPackage slave_cut_package;
		std::vector<SlaveCutPackage> all_package;

		gather(world, slave_cut_package, all_package, 0);

		for (int i(1); i < world.size(); i++) {
			sort_cut_slave(all_package[i]);
		}

		update_best_ub();

		_data.stop = stopping_criterion();
	}

	broadcast(world, _data.stop, 0);
	world.barrier();
}

/*!
*  \brief Add aggregated cut to Master Problem and store it in a set
*
*  Method to add aggregated cut from a slave to the Master Problem and store it in a map linking each slave to its set of cuts.
*
*  \param slave_cut_package : cut information
*/
void BendersMpi::sort_cut_slave_aggregate(SlaveCutPackage & slave_cut_package, Point & s, double & rhs) {
	for (auto & itmap : slave_cut_package) {

		SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
		handler->get_subgradient() = itmap.second.first.first.first;

		_data.ub += handler->get_dbl(SLAVE_COST) * _slave_weight_coeff[_problem_to_id[itmap.first]];
		rhs += handler->get_dbl(SLAVE_COST) * _slave_weight_coeff[_problem_to_id[itmap.first]];

		for (auto & var : s) {
			s[var.first] += handler->get_subgradient()[var.first]* _slave_weight_coeff[_problem_to_id[itmap.first]];
		}

		SlaveCutTrimmer cut(handler, _data.x0);

		if (!(_all_cuts_storage[itmap.first].find(cut) == _all_cuts_storage[itmap.first].end())) {
			_data.deletedcut++;
		}
		_all_cuts_storage.find(itmap.first)->second.insert(cut);

		if (_options.TRACE) {
			_trace._master_trace[_data.it - 1]->_cut_trace[itmap.first] = slave_cut_data;
		}

		bound_simplex_iter(handler->get_int(SIMPLEXITER));
	}
}

/*!
*  \brief Method to gather every cut information from each thread in thread 0 and add cut to Master problem
*
*/
void BendersMpi::step_3_aggregated(mpi::environment & env, mpi::communicator & world) {
	if (world.rank() == 0) {

		Point s;
		double rhs;
		for (auto & var : _data.x0) {
			s[var.first] = 0;
		}

		SlaveCutPackage slave_cut_package;
		std::vector<SlaveCutPackage> all_package;

		gather(world, slave_cut_package, all_package, 0);

		for (int i(1); i < world.size(); i++) {
			sort_cut_slave_aggregate(all_package[i], s, rhs);
		}

		_master->add_cut(s, _data.x0, rhs);

		update_best_ub();
		_data.stop = stopping_criterion();
	}

	broadcast(world, _data.stop, 0);
	world.barrier();
}

/*!
*  \brief Update trace of the Benders for the current iteration
*/
void BendersMpi::update_trace() {
	_trace._master_trace[_data.it - 1]->_lb = _data.lb;
	_trace._master_trace[_data.it - 1]->_ub = _data.ub;
	_trace._master_trace[_data.it - 1]->_bestub = _data.best_ub;
	_trace._master_trace[_data.it - 1]->_x0 = PointPtr(new Point(_data.x0));
	_trace._master_trace[_data.it - 1]->_deleted_cut = _data.deletedcut;
}

/*!
*  \brief Print iteration log
*
*  Method to print the log of an iteration
*
*  \param stream : output to print log
*/
void BendersMpi::print_log(std::ostream&stream) const {

	stream << std::setw(10) << _data.it;
	if (_data.lb == -1e20)
		stream << std::setw(20) << "-INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << _data.lb;
	if (_data.ub == +1e20)
		stream << std::setw(20) << "+INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << _data.ub;
	if (_data.best_ub == +1e20)
		stream << std::setw(20) << "+INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << _data.best_ub;

	if (_options.LOG_LEVEL > 1) {
		stream << std::setw(15) << _data.minsimplexiter;
		stream << std::setw(15) << _data.maxsimplexiter;
	}

	if (_options.LOG_LEVEL > 2) {
		stream << std::setw(15) << _data.deletedcut;
	}
	stream << std::endl;
}

/*!
*  \brief Print the optimal solution of the problem
*
*  Method to print the optimal solution of the problem
*
* \param stream : stream to print the output
*/
void BendersMpi::print_solution(std::ostream&stream)const {
	stream << std::endl;
	stream << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *" << std::endl;
	stream << "*                                                             *" << std::endl;
	stream << "*                     Investment solution                     *" << std::endl;
	stream << "*                                                             *" << std::endl;
	stream << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *" << std::endl;
	stream << "|                                                             |" << std::endl;
	for (auto const & kvp : _data.bestx) {
		stream << "|   " << std::setw(35) << std::left << kvp.first;
		stream << " = ";
		stream << std::setw(20) << std::scientific << std::setprecision(10) << kvp.second;
		stream << "|" << std::endl;
	}
	stream << "|_____________________________________________________________|" << std::endl;
	stream << std::endl;
}

/*!
*  \brief Update maximum and minimum of a set of int
*
*  \param simplexiter : int to compare to current max and min
*/
void BendersMpi::bound_simplex_iter(int simplexiter) {
	if (_data.maxsimplexiter < simplexiter) {
		_data.maxsimplexiter = simplexiter;
	}

	if (_data.minsimplexiter > simplexiter) {
		_data.minsimplexiter = simplexiter;
	}
}

/*!
*  \brief Update best upper bound and best optimal variables
*
*/
void BendersMpi::update_best_ub() {
	if (_data.best_ub > _data.ub) {
		_data.best_ub = _data.ub;
		_data.bestx = _data.x0;
	}
}

/*!
*  \brief Update stopping criterion
*
*  Method updating the stopping criterion and reinitializing some datas
*/
bool BendersMpi::stopping_criterion() {
	return(((_options.MAX_ITERATIONS != -1) && (_data.it > _options.MAX_ITERATIONS)) || (_data.lb + _options.GAP >= _data.best_ub));
}


void BendersMpi::free(mpi::environment & env, mpi::communicator & world) {
	if(world.rank() == 0)
		_master->free();
	else {
		for (auto & ptr : _map_slaves)
			ptr.second->free();
	}
	world.barrier();
}

/*!
*  \brief Initialize Benders data and log
*
*  Method to initialize Benders data and log by printing each column title
*
*  \param stream : output to print log
*/
void BendersMpi::init(mpi::environment & env, mpi::communicator & world, std::ostream & stream) {
	WorkerMaster & master(*_master);
	_data.lb = -1e20;
	_data.ub = +1e20;
	_data.x0.clear();
	_data.best_ub = +1e20;
	_data.stop = false;
	_data.it = 0;


	if (world.rank() == 0) {
		stream << std::setw(10) << "ITE";
		stream << std::setw(20) << "LB";
		stream << std::setw(20) << "UB";
		stream << std::setw(20) << "BESTUB";
		stream << std::setw(15) << "MINSIMPLEXIT";
		stream << std::setw(15) << "MAXSIMPLEXIT";
		stream << std::setw(15) << "DELETEDCUT";
		stream << std::endl;
		for (auto const & kvp : _problem_to_id) {
			_all_cuts_storage[kvp.first] = SlaveCutStorage();
		}
	}
}

/*!
*  \brief Print the trace of the Benders algorithm in a csv file
*
*  Method to print trace of the Benders algorithm in a csv file
*
* \param stream : stream to print the output
*/
void BendersMpi::print_csv() {
	std::string output(_options.ROOTPATH + PATH_SEPARATOR + "bendersMpi_output.csv");
	if (_options.AGGREGATION) {
		output = (_options.ROOTPATH + PATH_SEPARATOR + "bendersMpi_output_aggregate.csv");
	}
	std::ofstream file(output, std::ios::out | std::ios::trunc);

	if (file)
	{
		file << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;deletedcut" << std::endl;
		Point xopt;
		int nite;
		nite = _trace.get_ite();
		xopt = _trace._master_trace[nite - 1]->get_point();
		for (int i(0); i < nite; i++) {
			file << i + 1 << ";";
			file << "Master" << ";";
			file << "master" << ";";
			file << _data.nslaves << ";";
			file << _trace._master_trace[i]->get_ub() << ";";
			file << _trace._master_trace[i]->get_lb() << ";";
			file << _trace._master_trace[i]->get_bestub() << ";";
			file << norm_point(xopt, _trace._master_trace[i]->get_point()) << ";";
			file << _trace._master_trace[i]->get_deletedcut() << std::endl;
			for (auto & kvp : _trace._master_trace[i]->_cut_trace) {
				std::size_t found = kvp.first.find_last_of("/\\");
				SlaveCutDataHandler handler(kvp.second);
				file << i + 1 << ";";
				file << "Slave" << ";";
				file << kvp.first.substr(found + 1) << ";";
				file << _problem_to_id[kvp.first] << ";";
				file << handler.get_dbl(SLAVE_COST) << ";";
				file << handler.get_dbl(ALPHA_I) << ";";
				file << ";";
				file << handler.get_int(SIMPLEXITER) << ";";
				file << std::endl;
			}
		}
		file.close();
	}
	else {
		std::cout << "Impossible d'ouvrir le fichier .csv" << std::endl;
	}
}

/*!
*  \brief Run Benders algorithm in parallel 
*
*  Method to run Benders algorithm in parallel
*
* \param stream : stream to print the output
*/
void BendersMpi::run(mpi::environment & env, mpi::communicator & world, std::ostream & stream) {

	init(env, world, stream);

	world.barrier();

	while (!_data.stop) {
		++_data.it;
		_data.deletedcut = 0;

		/*Solve Master problem, get optimal value and cost and send it to Slaves*/
		step_1(env, world);

		/*Fix trial values in each slaves and send back data for Master to build cuts*/
		step_2(env, world);

		/*Receive datas from each slaves and add cuts to Master Problem*/
		if (_options.AGGREGATION == true) {
			step_3_aggregated(env, world);
		}
		else {
			step_3(env, world);
		}
		if (world.rank() == 0) {
			print_log(stream);
			if (_options.TRACE) {
				update_trace();
			}
		}

	}

	if (world.rank() == 0) {
		print_solution(stream);
		if (_options.TRACE) {
			print_csv();
		}
	}

}