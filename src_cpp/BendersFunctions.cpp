#include "BendersFunctions.h"

/*!
*  \brief Print the optimal solution of the problem
*
*  Method to print the optimal solution of the problem
*
* \param stream : stream to print the output
*
* \param point : point to print
*
* \param filter_non_zero : either if zeros coordinates need to be printed or not
*/
void print_solution(std::ostream&stream, Point const & point, bool filter_non_zero) {
	stream << std::endl;
	stream << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *" << std::endl;
	stream << "*                                                                                               *" << std::endl;
	stream << "*                                      Investment solution                                      *" << std::endl;
	stream << "*                                                                                               *" << std::endl;
	stream << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *" << std::endl;
	stream << "|                                                                                               |" << std::endl;
	for (auto const & kvp : point) {
		if (!filter_non_zero || std::fabs(kvp.second) > 1e-10) {
			stream << "|  " << std::setw(70) << std::left << kvp.first;
			stream << " = ";
			stream << std::setw(20) << std::scientific << std::setprecision(10) << kvp.second;
			stream << "|" << std::endl;
		}
	}
	stream << "|_______________________________________________________________________________________________|" << std::endl;
	stream << std::endl;
}

/*!
*  \brief Print iteration log
*
*  Method to print the log of an iteration
*
*  \param stream : output to print log
*
* \param data : data to print
*
* \param log_level : level of log precision (from 1 to 3)
*/
void print_log(std::ostream&stream, BendersData const & data, int const log_level) {

	stream << std::setw(10) << data.it;
	if (data.lb == -1e20)
		stream << std::setw(20) << "-INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << data.lb;
	if (data.ub == +1e20)
		stream << std::setw(20) << "+INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << data.ub;
	if (data.best_ub == +1e20)
		stream << std::setw(20) << "+INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << data.best_ub;
	stream << std::setw(15) << std::scientific << std::setprecision(2) << data.best_ub - data.lb;

	if (log_level > 1) {
		stream << std::setw(15) << data.minsimplexiter;
		stream << std::setw(15) << data.maxsimplexiter;
	}

	if (log_level > 2) {
		stream << std::setw(15) << data.deletedcut;
		stream << std::setw(15) << std::setprecision(2) << data.timer_master - data.timer_slaves;
		stream << std::setw(15) << std::setprecision(2) << data.timer_slaves;
	}
	stream << std::endl;

}

/*!
*  \brief Get the slave weight from a input file
*
*  Method to build the weight coefficients vector from an input file stored in the options
*
*  \param data : Benders data needed to init the vector construction
*
* \param options : parameters needed to fill the weight vector
*
* \param problem_to_id : map linking each problem to its id
*/
void init_slave_weight(BendersData const & data, BendersOptions const & options, DblVector & slave_weight_coeff, std::map< std::string, int> & problem_to_id) {
	slave_weight_coeff.resize(data.nslaves);
	for (auto const & kvp : problem_to_id) {
		slave_weight_coeff[kvp.second] = options.slave_weight(data.nslaves, kvp.first);
	}
}

/*!
*  \brief Initialize Benders log
*
*  Method to initialize Benders log by printing each column title
*
*  \param stream : output to print log
*
* \param log_level : level of log precision (from 1 to 3)
*/
void init_log(std::ostream&stream, int const log_level) {
	stream << std::setw(10) << "ITE";
	stream << std::setw(20) << "LB";
	stream << std::setw(20) << "UB";
	stream << std::setw(20) << "BESTUB";
	stream << std::setw(15) << "GAP";

	if (log_level > 1) {
		stream << std::setw(15) << "MINSIMPLEX";
		stream << std::setw(15) << "MAXSIMPLEX";
	}

	if (log_level > 2) {
		stream << std::setw(15) << "DELETEDCUT";
		stream << std::setw(15) << "TIMEMASTER";
		stream << std::setw(15) << "TIMESLAVES";
	}
	stream << std::endl;
}

/*!
*  \brief Update best upper bound and best optimal variables
*
*	Function to update best upper bound and best optimal variables regarding the current ones
*
*  \param best_ub : current best upper bound
*
*  \param ub : current upper bound
*
*  \param bestx : current best optimal variables
*
*  \param x0 : current optimal variables
*/
void update_best_ub(double & best_ub, double const & ub, Point & bestx, Point const & x0) {
	if (best_ub > ub) {
		best_ub = ub;
		bestx = x0;
	}
}

/*!
*	\brief Update maximum and minimum of a set of int
*
*	\param simplexiter : int to compare to current max and min
*
*   \param data : data containing current maximum and minimum integer
*/
void bound_simplex_iter(int simplexiter, BendersData & data) {
	if (data.maxsimplexiter < simplexiter) {
		data.maxsimplexiter = simplexiter;
	}
	if (data.minsimplexiter > simplexiter) {
		data.minsimplexiter = simplexiter;
	}
}

/*!
*  \brief Update stopping criterion
*
*  Method updating the stopping criterion and reinitializing some datas
*
*  \param data : data containing benders information
*
*  \param options : stopping parameters
*/
bool stopping_criterion(BendersData & data, BendersOptions const & options) {
	data.deletedcut = 0;
	data.maxsimplexiter = 0;
	data.minsimplexiter = std::numeric_limits<int>::max();
	return(((options.MAX_ITERATIONS != -1) && (data.it > options.MAX_ITERATIONS)) || (data.lb + options.GAP >= data.best_ub));
}

/*!
*  \brief Solve and get optimal variables of the Master Problem
*
*  Method to solve and get optimal variables of the Master Problem and update upper and lower bound
*
*  \param master : pointer to the master problem
*
*  \param data : benders data to update with master optimal solution
*/
void get_master_value(WorkerMasterPtr & master, BendersData & data, BendersOptions const & options) {
	Timer timer_master;
	data.alpha_i.resize(data.nslaves);
	master->fix_alpha(data.best_ub);
	master->solve(data.master_status);
	master->get(data.x0, data.alpha, data.alpha_i); /*Get the optimal variables of the Master Problem*/
	master->get_value(data.lb); /*Get the optimal value of the Master Problem*/
	data.invest_cost = data.lb - data.alpha;
	if (!options.RAND_AGGREGATION) {
		data.ub = data.invest_cost;
	}
	data.timer_master = timer_master.elapsed();
	data.nconstraint = master->get_number_constraint();
}


/*!
*  \brief Solve and store optimal variables of a Slave Problem
*
*  Method to solve and store optimal variables of a Slave Problem after fixing trial values
*
*  \param slave_cut_package : map storing for each slave its cut
*
*  \param map_slaves : map linking each problem name to its problem
*
*  \param data : data containing trial values
*
*  \param options : set of parameters
*/
void get_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, BendersOptions const & options, BendersData const & data) {
	for (auto & kvp : map_slaves) {
		Timer timer_slave;
		WorkerSlavePtr & ptr(kvp.second);
		IntVector intParam(SlaveCutInt::MAXINT);
		DblVector dblParam(SlaveCutDbl::MAXDBL);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

		ptr->fix_to(data.x0);
		ptr->solve(handler->get_int(LPSTATUS));
		if (options.BASIS) {
			ptr->get_basis();
		}
		ptr->get_value(handler->get_dbl(SLAVE_COST));
		ptr->get_subgradient(handler->get_subgradient());
		ptr->get_simplex_ite(handler->get_int(SIMPLEXITER));
		handler->get_dbl(SLAVE_TIMER) = timer_slave.elapsed();
		slave_cut_package[kvp.first] = *slave_cut_data;
	}
}

void get_random_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, std::set<std::string> const & random_slaves, BendersOptions const & options, BendersData const & data) {
	for (auto & kvp : random_slaves) {
		if (map_slaves.find(kvp) != map_slaves.end()) {
			Timer timer_slave;
			WorkerSlavePtr & ptr(map_slaves[kvp]);
			IntVector intParam(SlaveCutInt::MAXINT);
			DblVector dblParam(SlaveCutDbl::MAXDBL);
			SlaveCutDataPtr slave_cut_data(new SlaveCutData);
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

			ptr->fix_to(data.x0);
			ptr->solve(handler->get_int(LPSTATUS));
			if (options.BASIS) {
				ptr->get_basis();
			}
			ptr->get_value(handler->get_dbl(SLAVE_COST));
			ptr->get_subgradient(handler->get_subgradient());
			ptr->get_simplex_ite(handler->get_int(SIMPLEXITER));
			handler->get_dbl(SLAVE_TIMER) = timer_slave.elapsed();
			slave_cut_package[kvp] = *slave_cut_data;
		}
	}
}

/*!
*  \brief Get all slaves basis from a map
*
*  Fonction to get all slaves basis
*
*  \param simplex_basis_package : map linking each slave to its current simplex basis
*
*  \param map_slaves : map linking each slaves names to their problem
*/
void get_slave_basis(SimplexBasisPackage & simplex_basis_package, SlavesMapPtr & map_slaves) {
	for (auto & kvp : map_slaves) {
		WorkerSlavePtr & ptr(kvp.second);
		simplex_basis_package[kvp.first] = ptr->get_basis();
	}
}

/*!
*  \brief Store all cuts status at each iteration
*
*  Fonction to store all cuts status from master problem at each iteration
*
*  \param master : pointer to master problem
*
*  \param active_cuts : vector of tuple storing each cut status
*
*  \param cut_id : map linking each cut from each slave to its id in master problem
*
*  \param it : current iteration
*/
void update_active_cuts(WorkerMasterPtr & master, std::vector<ActiveCut> & active_cuts, SlaveCutId & cut_id, int const it) {
	std::vector<double> dual;
	master->get_dual_values(dual);
	for (auto & kvp : cut_id) {
		for (int i(0); i < kvp.second.size(); i++) {
			active_cuts.push_back(std::make_tuple(it, kvp.first, i+1, (dual[kvp.first[i]] != 0)));
		}
	}
}

/*!
*  \brief Store all slaves' basis in a set
*
*  Fonction to store and sort all slaves' basis in a set
*
*  \param all_basis_package : vector of slaves basis
*
*  \param problem_to_id : map linking each problem name to its id
*
*  \param basis_storage : set storing every basis
*
*  \param data : set of Benders data
*/
void sort_basis(std::vector<SimplexBasisPackage> const & all_basis_package, std::map<std::string, int> & problem_to_id, std::set<SimplexBasisHandler> & basis_storage, BendersData & data) {
	for (int i(0); i < all_basis_package.size(); i++) {
		for (auto & itmap : all_basis_package[i]) {
			SimplexBasisPtr basis(new SimplexBasis(itmap.second));
			SimplexBasisHandler handler(basis);
			basis_storage.insert(handler);
		}
	}
	data.nbasis = basis_storage.size();
}

/*!
*  \brief Update trace of the Benders for the current iteration
*
*  Fonction to store the current Benders data in the trace
*
*  \param trace : vector keeping data for each iteration
*
*  \param data : data to store
*/
void update_trace(std::vector<WorkerMasterDataPtr> trace, BendersData const & data) {
	trace[data.it - 1]->_lb = data.lb;
	trace[data.it - 1]->_ub = data.ub;
	trace[data.it - 1]->_bestub = data.best_ub;
	trace[data.it - 1]->_x0 = PointPtr(new Point(data.x0));
	trace[data.it - 1]->_deleted_cut = data.deletedcut;
	trace[data.it - 1]->_time = data.timer_master;
	trace[data.it - 1]->_nbasis = data.nbasis;

}

/*!
*  \brief Initialize set of data used in the loop
*
*  \param data : Benders data 
*/
void init(BendersData & data) {
	std::srand(time(NULL));
	data.nbasis = 0;
	data.lb = -1e20;
	data.ub = +1e20;
	data.best_ub = +1e20;
	data.stop = false;
	data.it = 0;
	data.alpha = 0;
	data.invest_cost = 0;
	data.deletedcut = 0;
	data.maxsimplexiter = 0;
	data.minsimplexiter = std::numeric_limits<int>::max();
}

/*!
*  \brief Add cut to Master Problem and store the cut in a set
*
*  Method to add cut from a slave to the Master Problem and store this cut in a map linking each slave to its set of cuts.
*
*  \param all_package : vector storing all cuts information for each slave problem
*
*  \param slave_weight_coeff : vector linking each slave id to its weight in the master problem
*
*  \param master : pointer to thte master problem
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param trace : vector keeping data for each iteration
*
*  \param all_cuts_storage : set to store every new cut
*
*  \param slave_cut_id : map linking each cut to its id in the master problem 
*
*  \param data : Benders data 
*
*  \param options : set of parameters
*
*/
void sort_cut_slave(std::vector<SlaveCutPackage> const & all_package, WorkerMasterPtr & master, std::map<std::string, int> & problem_to_id, std::vector<WorkerMasterDataPtr> & trace, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options, SlaveCutId & slave_cut_id) {
	for (int i(0); i < all_package.size(); i++) {
		for (auto & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			handler->get_dbl(ALPHA_I) = data.alpha_i[problem_to_id[itmap.first]];
			data.ub += handler->get_dbl(SLAVE_COST);
			SlaveCutTrimmer cut(handler, data.x0);
			if (options.DELETE_CUT && !(all_cuts_storage[itmap.first].find(cut) == all_cuts_storage[itmap.first].end())) {
				data.deletedcut++;
			}
			else {
				master->add_cut_slave(problem_to_id[itmap.first], handler->get_subgradient(), data.x0, handler->get_dbl(SLAVE_COST));
				if (options.ACTIVECUTS) {
					slave_cut_id[itmap.first].push_back(data.nconstraint);
					data.nconstraint++;
				}
				all_cuts_storage[itmap.first].insert(cut);
			}
			if (options.TRACE) {
				trace[data.it - 1]->_cut_trace[itmap.first] = slave_cut_data;
			}
			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);
		}
	}
}

/*!
*  \brief Add aggregated cut to Master Problem and store it in a set
*
*  Method to add aggregated cut from slaves to Master Problem and store it in a map linking each slave to its set of non-aggregated cut
*
*  \param all_package : vector storing all cuts information for each slave problem
*
*  \param slave_weight_coeff : vector linking each slave id to its weight in the master problem
*
*  \param master : pointer to thte master problem
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param trace : vector keeping data for each iteration
*
*  \param all_cuts_storage : set to store every new cut
*
*  \param data : Benders data
*
*  \param options : set of parameters
*/
void sort_cut_slave_aggregate(std::vector<SlaveCutPackage> const & all_package, DblVector const & slave_weight_coeff, WorkerMasterPtr & master, std::map<std::string, int> & problem_to_id, std::vector<WorkerMasterDataPtr> & trace, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options) {
	Point s;
	double rhs(0);
	for (int i(0); i < all_package.size(); i++) {
		for (auto & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

			data.ub += handler->get_dbl(SLAVE_COST) * slave_weight_coeff[problem_to_id[itmap.first]];
			rhs += handler->get_dbl(SLAVE_COST) * slave_weight_coeff[problem_to_id[itmap.first]];

			for (auto & var : data.x0) {
				s[var.first] += handler->get_subgradient()[var.first] * slave_weight_coeff[problem_to_id[itmap.first]];
			}

			SlaveCutTrimmer cut(handler, data.x0);

			if (options.DELETE_CUT && !(all_cuts_storage[itmap.first].find(cut) == all_cuts_storage[itmap.first].end())) {
				data.deletedcut++;
			}
			all_cuts_storage.find(itmap.first)->second.insert(cut);

			if (options.TRACE) {
				trace[data.it - 1]->_cut_trace[itmap.first] = slave_cut_data;
			}

			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);
		}
	}
	master->add_cut(s, data.x0, rhs);
}

/*!
*  \brief Print the trace of the Benders algorithm in a csv file
*
*  Method to print trace of the Benders algorithm in a csv file
*
* \param stream : stream to print the output
*
* \param problem_to_id : map linking each problem name to its id
*
* \param data : set of Benders data
*
* \param options : set of parameters
*/
void print_csv(std::vector<WorkerMasterDataPtr> & trace, std::map<std::string, int> & problem_to_id, BendersData const & data, BendersOptions const & options) {
	 std::string output(options.OUTPUTROOT + PATH_SEPARATOR + "benders_output.csv");
	 if (options.AGGREGATION) {
		 output = (options.OUTPUTROOT + PATH_SEPARATOR + "benders_output_aggregate.csv");
	 }
	 std::ofstream file(output, std::ios::out | std::ios::trunc);
	 if (file)
	 {
		 file << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;jump;alpha_i;deletedcut;time;basis;" << std::endl;
		 Point xopt;
		 int nite;
		 nite = trace.size();
		 xopt = trace[nite - 1]->get_point();
		 file << 1 << ";";
		 print_master_csv(file, trace[0], xopt, options.MASTER_NAME, data.nslaves);
		 for (auto & kvp : trace[0]->_cut_trace) {
			 SlaveCutDataHandler handler(kvp.second);
			 file << 1 << ";";
			 print_cut_csv(file, handler, kvp.first, problem_to_id[kvp.first]);
		 }
		 for (int i(1); i < nite; i++) {
			 file << i + 1 << ";";
			 print_master_csv(file, trace[i], trace[i-1]->get_point(), options.MASTER_NAME, data.nslaves);
			 for (auto & kvp : trace[i]->_cut_trace) {
				 SlaveCutDataHandler handler(kvp.second);
				 file << i + 1 << ";";
				 print_cut_csv(file, handler, kvp.first, problem_to_id[kvp.first]);
			 }
		 }
		 file.close();
	 }
	 else {
		 std::cout << "Impossible d'ouvrir le fichier .csv" << std::endl;
	 }
}

/*!
*  \brief Print in a file master's information
*
*  \param stream : output stream
*
*  \param trace : storage of problem data
*
*  \param xopt : final optimal value
*
*  \param name : master name
*
*  \param nslaves : number of slaves
*/
void print_master_csv(std::ostream&stream, WorkerMasterDataPtr & trace, Point const & xopt, std::string const & name, int const nslaves) {
	stream << "Master" << ";";
	stream << name << ";";
	stream << nslaves << ";";
	stream << trace->get_ub() << ";";
	stream << trace->get_lb() << ";";
	stream << trace->get_bestub() << ";";
	stream << ";";
	stream << norm_point(xopt, trace->get_point()) << ";";
	stream << ";";
	stream << trace->get_deletedcut() << ";";
	stream << trace->_time << ";";
	stream << trace->_nbasis << ";" << std::endl;
}

/*!
*  \brief Print in a file slave's information
*
*  \param stream : output stream
*
*  \param handler : handler to manage slave data
*
*  \param name : problem name
*
*  \param islaves : problem id
*/
void print_cut_csv(std::ostream&stream, SlaveCutDataHandler const & handler, std::string const & name, int const islaves) {
	stream << "Slave" << ";";
	stream << name << ";";
	stream << islaves << ";";
	stream << handler.get_dbl(SLAVE_COST) << ";";
	stream << ";";
	stream << ";";
	stream << handler.get_int(SIMPLEXITER) << ";";
	stream << ";";
	stream << handler.get_dbl(ALPHA_I) << ";";
	stream << ";";
	stream << handler.get_dbl(SLAVE_TIMER) << ";";
	stream << std::endl;
}

/*!
*  \brief Check if every slave has been solved to optimality
*
*  \param all_package : storage of each slaves status
*/
void check_status(std::vector<SlaveCutPackage> const & all_package, BendersData const & data) {
	if (data.master_status != 1) {
		std::cout << "Master status is " << data.master_status << std::endl;
	}
	for (int i(0); i < all_package.size(); i++) {
		for (auto & kvp : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(kvp.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			if (handler->get_int(LPSTATUS) != 1) {
				std::cout << "Slave " << kvp.first << " status is " << handler->get_int(LPSTATUS) << std::endl;
			}
		}
	}
}

/*!
*  \brief Write every information needed to start back in a file
*
*	Function to print each cut, current lower bound and upper bound to start back from where the algorithm stopped
*
*  \param cut_storage : storage of all cuts to print
*
*  \param data : Benders data to print
*
*  \param options : set of parameters
*/
void dump_cut(AllCutStorage const & cut_storage, BendersData const & data, BendersOptions const & options)
{
	std::string output(options.OUTPUTROOT + PATH_SEPARATOR + "benders_dump_output.txt");
	std::ofstream file(output, std::ios::out | std::ios::trunc);
	if (file) {
		file << std::setw(15) << "AGGREGATION" << std::setw(20) << options.AGGREGATION << std::endl;
		file << std::setw(15) << "UB" << std::setw(20) << data.ub << std::endl;
		file << std::setw(15) << "LB" << std::setw(20) << data.lb << std::endl;
		for (auto & kvp : cut_storage) {
			for (auto & itset : kvp.second) {
				file << std::setw(15) << kvp.first;
				file << std::setw(50) << itset._x0;
				file << std::setw(15) << itset._data_cut->get_dbl(SLAVE_COST);
				file << std::setw(50) << itset._data_cut->get_subgradient() << std::endl;
			}
		}
		file.close();
	}
}

/*!
*  \brief Write in a csv file every active cut at every iteration
*
*	Write in a csv file every active cut for every slave for all iterations
*
*  \param active_cuts : vector of tuple storing which cut is active or not 
*
*  \param options : set of parameters
*/
void print_active_cut(std::vector<ActiveCut> const & active_cuts, BendersOptions const & options) {
	std::string output(options.OUTPUTROOT + PATH_SEPARATOR + "active_cut_output.csv");
	std::ofstream file(output, std::ios::out | std::ios::trunc);
	if (file)
	{
		file << "Ite;Slave;CutNumber;IsActive;" << std::endl;
		for (int i(0); i < active_cuts.size(); i++) {
			file << std::get<0>(active_cuts[i]) << ";";
			file << std::get<1>(active_cuts[i]) << ";";
			file << std::get<2>(active_cuts[i]) << ";";
			file << std::get<3>(active_cuts[i]) << ";" << std::endl;
		}
		file.close();
	}
	else {
		std::cout << "Impossible d'ouvrir le fichier .csv" << std::endl;
	}
}

/*!
*  \brief Store the current aggregate cuts for further aggregation
*
*	Store the current aggregate cuts in case of partial aggregation
*
*  \param dynamic_cuts : vector of tuple storing cut information (rhs, x0, subgradient)
*
*  \param all_package : storage of every slave information
*
*  \param slave_weight_coeff : vector linking each slave id to its weight in the master problem
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param x0 : trial values fixed in each slave
*/
void store_current_aggregate_cut(DynamicAggregateCuts & dynamic_cuts, std::vector<SlaveCutPackage> const & all_package, DblVector const & slave_weight_coeff, std::map<std::string, int> problem_to_id, Point const & x0) {
	Point s;
	double rhs(0);
	for (int i(0); i < all_package.size(); i++) {
		for (auto & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			for (auto & kvp : handler->get_subgradient()) {
				s[kvp.first] += kvp.second * slave_weight_coeff[problem_to_id[itmap.first]];
			}
			rhs += handler->get_dbl(SLAVE_COST) * slave_weight_coeff[problem_to_id[itmap.first]];
		}
	}
	dynamic_cuts.push_back(std::tuple<Point, Point, double>(s, x0, rhs));

}

/*!
*  \brief Add all the stored cuts in case of partial aggregation
*
*	Delete every previous simple cut and replace them with the aggregated ones
*
*  \param dynamic_cuts : vector of tuple storing cut information (rhs, x0, subgradient)
*
*  \param master : pointer to master problem
*
*  \param it : number of iteration
*
*  \param nconstraints : number of previous added constraints to delete
*/
void gather_cut(DynamicAggregateCuts & dynamic_cuts, WorkerMasterPtr & master, int const it, int const nconstraints) {
	master->delete_constraint(nconstraints);
	for (int i(0); i < dynamic_cuts.size(); i++) {
		master->add_cut(std::get<0>(dynamic_cuts[i]), std::get<1>(dynamic_cuts[i]), std::get<2>(dynamic_cuts[i]));
	}
	dynamic_cuts.clear();
}

/*!
*  \brief Select a set of random slaves
*
*	Select a set of random slaves in the map of problem
*
*  \param problem_to_id : map linking each slaves to its id
*
*  \param options : set of benders options
*
*  \param random_slaves : set of random slaves selected
*/
void select_random_slaves(std::map<std::string, int> & problem_to_id, BendersOptions const & options, std::set<std::string> & random_slaves) {
	while (random_slaves.size() < options.RAND_AGGREGATION) {
		auto it = problem_to_id.begin();
		std::advance(it, std::rand() % problem_to_id.size());
		random_slaves.insert(it->first);
	}
}

/*!
*  \brief Add the random cuts in master problem
*
*	Add the random cuts in master problem
*
*  \param master : pointer to master problem
*
*  \param all_package : storage of every slave information
*
*  \param slave_weight_coeff : vector linking each slave id to its weight in the master problem
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param random_slaves : set of random slaves selected
*
*  \param trace : vector keeping data for each iteration
*
*  \param options : set of benders options
*
*  \param data : set of benders data
*/
void add_random_cuts(WorkerMasterPtr & master, std::vector<SlaveCutPackage> const & all_package, DblVector const & slave_weight_coeff, std::map<std::string, int> & problem_to_id, std::set<std::string> & random_slaves, std::vector<WorkerMasterDataPtr> & trace, BendersOptions & options, BendersData & data) {
	int nboundslaves(0);
	for (int i(0); i < all_package.size(); i++) {
		for (auto & kvp : random_slaves) {
			if (all_package[i].find(kvp) != all_package[i].end()) {
				SlaveCutDataPtr slave_cut_data(new SlaveCutData(all_package[i].find(kvp)->second));
				SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
				master->add_cut_slave(problem_to_id[kvp], handler->get_subgradient(), data.x0, handler->get_dbl(SLAVE_COST));
				handler->get_dbl(ALPHA_I) = data.alpha_i[problem_to_id[kvp]];
				bound_simplex_iter(handler->get_int(SIMPLEXITER), data);
				if (handler->get_dbl(SLAVE_COST) <= options.GAP + handler->get_dbl(ALPHA_I)) {
					nboundslaves++;
				}
				if (options.TRACE) {
					trace[data.it - 1]->_cut_trace[kvp] = slave_cut_data;
				}
				random_slaves.erase(kvp);
			}
		}
	}
	if (nboundslaves == options.RAND_AGGREGATION) {
		options.RAND_AGGREGATION = 0;
	}
}

/*!
*  \brief Add the random aggregated cuts in master problem
*
*	Add the random aggregated cuts in master problem
*
*  \param master : pointer to master problem
*
*  \param all_package : storage of every slave information
*
*  \param slave_weight_coeff : vector linking each slave id to its weight in the master problem
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param random_slaves : set of random slaves selected
*
*  \param trace : vector keeping data for each iteration
*
*  \param options : set of benders options
*
*  \param data : set of benders data
*/
void add_random_aggregate_cuts(WorkerMasterPtr & master, std::vector<SlaveCutPackage> const & all_package, DblVector const & slave_weight_coeff, std::map<std::string, int> & problem_to_id, std::set<std::string> & random_slaves, std::vector<WorkerMasterDataPtr> & trace, BendersOptions & options, BendersData & data) {
	Point s;
	double rhs(0);
	int nboundslaves(0);
	IntVector slaves_id;
	for (int i(0); i < all_package.size(); i++) {
		for (auto & kvp : random_slaves) {
			if (all_package[i].find(kvp) != all_package[i].end()) {
				SlaveCutDataPtr slave_cut_data(new SlaveCutData(all_package[i].find(kvp)->second));
				SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
				handler->get_dbl(ALPHA_I) = data.alpha_i[problem_to_id[kvp]];
				rhs += handler->get_dbl(SLAVE_COST) * slave_weight_coeff[problem_to_id[kvp]];
				bound_simplex_iter(handler->get_int(SIMPLEXITER), data);
				for (auto & var : data.x0) {
					s[var.first] += handler->get_subgradient()[var.first] * slave_weight_coeff[problem_to_id[kvp]];
				}
				if (handler->get_dbl(SLAVE_COST) <= options.GAP + handler->get_dbl(ALPHA_I)) {
					nboundslaves++;
				}
				slaves_id.push_back(problem_to_id[kvp]);
				if (options.TRACE) {
					trace[data.it - 1]->_cut_trace[kvp] = slave_cut_data;
				}
				random_slaves.erase(kvp);
			}
		}
	}
	if (nboundslaves == options.RAND_AGGREGATION) {
		options.RAND_AGGREGATION = 0;
	}
	master->add_random_cut(slaves_id, slave_weight_coeff, s, data.x0, rhs);
}

/*!
*  \brief Add cuts in master problem
*
*	Add cuts in master problem according to the selected option
*
*  \param master : pointer to master problem
*
*  \param all_package : storage of every slave information
*
*  \param slave_weight_coeff : vector linking each slave id to its weight in the master problem
*
*  \param problem_to_id : map linking each problem to its id
*
*  \param random_slaves : set of random slaves selected
*
*  \param trace : vector keeping data for each iteration
*
*  \param slave_cut_id : map linking each slaves to their cuts ids in the master problem
*
*  \param all_cuts_storage : set to store every new cut
*
*  \param dynamic_aggregate_cuts : vector of tuple storing cut information (rhs, x0, subgradient)
*
*  \param options : set of benders options
*
*  \param data : set of benders data
*/
void build_cut_full(WorkerMasterPtr & master, std::vector<SlaveCutPackage> const & all_package, Str2Int & problem_to_id, std::set<std::string> & random_slaves, std::vector<WorkerMasterDataPtr> & trace, SlaveCutId & slave_cut_id, AllCutStorage & all_cuts_storage, DynamicAggregateCuts & dynamic_aggregate_cuts, BendersData & data, BendersOptions & options) {
	//check_status(all_package, data);
	//if (!options.AGGREGATION && !options.RAND_AGGREGATION) {
	//	sort_cut_slave(all_package, master, problem_to_id, trace, all_cuts_storage, data, options, slave_cut_id);
	//}
	//else if (options.AGGREGATION && !options.RAND_AGGREGATION) {
	//	sort_cut_slave_aggregate(all_package, slave_weight_coeff, master, problem_to_id, trace, all_cuts_storage, data, options);
	//}
	//else if (!options.AGGREGATION && options.RAND_AGGREGATION) {
	//	add_random_cuts(master, all_package, slave_weight_coeff, problem_to_id, random_slaves, trace, options, data);
	//}
	//else if (options.AGGREGATION && options.RAND_AGGREGATION) {
	//	add_random_aggregate_cuts(master, all_package, slave_weight_coeff, problem_to_id, random_slaves, trace, options, data);
	//}
	//if (options.THRESHOLD_AGGREGATION > 1) {
	//	store_current_aggregate_cut(dynamic_aggregate_cuts, all_package, slave_weight_coeff, problem_to_id, data.x0);
	//	if (data.it % options.THRESHOLD_AGGREGATION == 0) {
	//		gather_cut(dynamic_aggregate_cuts, master, data.it, options.THRESHOLD_AGGREGATION * data.nslaves);
	//	}
	//}
}
