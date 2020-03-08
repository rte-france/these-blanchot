#include "BendersFunctions.h"


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
	
	// Solver status
	data.global_prb_status = 0;
	data.master_status = 0;
	data.slave_status = 0;

	// in-out stab
	data.stab_value = 0.5;
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
void print_solution(std::ostream&stream, Point const & point, bool const filter_non_zero, int status) {
	if (status == OPTIMAL) {
		stream << std::endl;
		stream << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *" << std::endl;
		stream << "*                                                                                               *" << std::endl;
		stream << "*                                      Investment solution                                      *" << std::endl;
		stream << "*                                                                                               *" << std::endl;
		stream << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *" << std::endl;
		stream << "|                                                                                               |" << std::endl;
		for (auto const& kvp : point) {
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
	else if(status == INFEASIBLE){
		std::cout << "****************************************************" << std::endl;
		std::cout << "               THE PROBLEM IS INFEASIBLE			  " << std::endl;
		std::cout << "****************************************************" << std::endl;
	}
	else if (status == UNBOUNDED) {
		std::cout << "****************************************************" << std::endl;
		std::cout << "               THE PROBLEM IS UNBOUNDED			  " << std::endl;
		std::cout << "****************************************************" << std::endl;
	}
	else {
		std::cout << "****************************************************" << std::endl;
		std::cout << "  ERROR : PROBLEM STATUS IS UNKNOWN			      " << std::endl;
		std::cout << "****************************************************" << std::endl;
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
void print_active_cut(ActiveCutStorage const & active_cuts, BendersOptions const & options) {
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
	return(
		((options.MAX_ITERATIONS != -1) && (data.it > options.MAX_ITERATIONS)) || 
		(data.lb + options.GAP >= data.best_ub) ||
		(data.global_prb_status != 0)
		);
}

/*!
*  \brief Check if every slave has been solved to optimality
*
*  \param all_package : storage of each slaves status
*/
void check_status(AllCutPackage const & all_package, BendersData & data) {
	if (data.master_status != OPTIMAL) {
		std::cout << "Master status is " << data.master_status << std::endl;
		data.global_prb_status = data.master_status;
	}
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & kvp : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(kvp.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			if (handler->get_int(LPSTATUS) != OPTIMAL) {
				std::cout << "Slave " << kvp.first << " status is " << handler->get_int(LPSTATUS) << std::endl;
				data.global_prb_status = data.slave_status;
			}
		}
	}
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

	master->solve_integer(data.master_status, options, "master_");
	
	master->get(data.x0, data.alpha, data.alpha_i); /*Get the optimal variables of the Master Problem*/
	master->get_value(data.lb); /*Get the optimal value of the Master Problem*/

	/*data.invest_cost = data.lb - data.alpha;
	
	if (!options.RAND_AGGREGATION) {
		data.ub = data.invest_cost;
	}*/
	data.timer_master = timer_master.elapsed();
}


/*!
*  \brief Solve and store optimal variables of all Slaves Problems
*
*  Method to solve and store optimal variables of all Slaves Problems after fixing trial values
*
*  \param slave_cut_package : map storing for each slave its cut
*
*  \param map_slaves : map linking each problem name to its problem
*
*  \param data : data containing trial values
*
*  \param options : set of parameters
*/
int get_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, BendersOptions const & options, BendersData const & data) {
	
	// Store the status of a non  optimal slave, 0 if all opitmals
	int slaves_worth_status = 0;

	for (auto & kvp : map_slaves) {
		Timer timer_slave;
		WorkerSlavePtr & ptr(kvp.second);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
		ptr->fix_to(data.x_cut);
		ptr->solve(handler->get_int(LPSTATUS), options, kvp.first);

		slaves_worth_status = std::max(slaves_worth_status, handler->get_int(LPSTATUS));

		ptr->get_value(handler->get_dbl(SLAVE_COST));
		ptr->get_subgradient(handler->get_subgradient());
		ptr->get_simplex_ite(handler->get_int(SIMPLEXITER));
		handler->get_dbl(SLAVE_TIMER) = timer_slave.elapsed();
		slave_cut_package[kvp.first] = *slave_cut_data;
	}
	return slaves_worth_status;
}

/*!
*  \brief Solve and store optimal variables of random Slaves Problems
*
*  Method to solve and store optimal variables of random Slaves Problems after fixing trial values
*
*  \param slave_cut_package : map storing for each slave its cut
*
*  \param map_slaves : map linking each problem name to its problem
*
*  \param data : data containing trial values
*
*  \param options : set of parameters
*/
int get_random_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, StrVector const & random_slaves, BendersOptions const & options, BendersData const & data) {
	// Store the status of a non  optimal slave, 0 if all opitmals
	int slaves_worth_status = 0;
	
	for (int i(0); i < data.nrandom; i++){
		Timer timer_slave;
		std::string const name_slave(random_slaves[i]);
		WorkerSlavePtr & ptr(map_slaves[name_slave]);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
		ptr->fix_to(data.x_cut);
		ptr->solve(handler->get_int(LPSTATUS), options, name_slave);

		slaves_worth_status = std::max(slaves_worth_status, handler->get_int(LPSTATUS));

		ptr->get_value(handler->get_dbl(SLAVE_COST));
		ptr->get_subgradient(handler->get_subgradient());
			ptr->get_simplex_ite(handler->get_int(SIMPLEXITER));
		handler->get_dbl(SLAVE_TIMER) = timer_slave.elapsed();
		slave_cut_package[name_slave] = *slave_cut_data;
	}
	return slaves_worth_status;
}

/*!
*  \brief Add cut to Master Problem and store the cut in a set
*
*  Method to add cut from a slave to the Master Problem and store this cut in a map linking each slave to its set of cuts.
*
*  \param all_package : vector storing all cuts information for each slave problem
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
void sort_cut_slave(AllCutPackage const & all_package, WorkerMasterPtr & master, Str2Int & problem_to_id, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options, SlaveCutId & slave_cut_id) {
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			handler->get_dbl(ALPHA_I) = data.alpha_i[problem_to_id[itmap.first]];
			data.ub += handler->get_dbl(SLAVE_COST);
			SlaveCutTrimmer cut(handler, data.x_cut);
			if (options.DELETE_CUT && !(all_cuts_storage[itmap.first].find(cut) == all_cuts_storage[itmap.first].end())) {
				data.deletedcut++;
			}
			else {
				master->add_cut_slave(problem_to_id[itmap.first], handler->get_subgradient(), data.x0, handler->get_dbl(SLAVE_COST));
				if (options.ACTIVECUTS) {
					slave_cut_id[itmap.first].push_back(master->get_number_constraint());
				}
				all_cuts_storage[itmap.first].insert(cut);
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
void sort_cut_slave_aggregate(AllCutPackage const & all_package, WorkerMasterPtr & master, Str2Int & problem_to_id, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options) {
	Point s;
	double rhs(0);
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			data.ub += handler->get_dbl(SLAVE_COST);
			rhs += handler->get_dbl(SLAVE_COST);
			for (auto const & var : data.x_cut) {
				s[var.first] += handler->get_subgradient()[var.first];
			}
			SlaveCutTrimmer cut(handler, data.x_cut);
			if (options.DELETE_CUT && !(all_cuts_storage[itmap.first].find(cut) == all_cuts_storage[itmap.first].end())) {
				data.deletedcut++;
			}
			all_cuts_storage.find(itmap.first)->second.insert(cut);

			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);
		}
	}
	master->add_cut(s, data.x0, rhs);
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
*  \param problem_to_id : map linking each problem to its id
*
*  \param trace : vector keeping data for each iteration
*
*  \param options : set of benders options
*
*  \param data : set of benders data
*/
void add_random_cuts(WorkerMasterPtr & master, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersOptions & options, BendersData & data) {
	int nboundslaves(0);
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & kvp : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(kvp.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			master->add_cut_slave(problem_to_id[kvp.first], handler->get_subgradient(), data.x0, handler->get_dbl(SLAVE_COST));
			handler->get_dbl(ALPHA_I) = data.alpha_i[problem_to_id[kvp.first]];
			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);
			if (handler->get_dbl(SLAVE_COST) <= options.GAP + handler->get_dbl(ALPHA_I)) {
				nboundslaves++;
			}

		}
	}
	if (nboundslaves == options.RAND_AGGREGATION) {
		options.RAND_AGGREGATION = 0;
	}
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
*  \param problem_to_id : map linking each problem to its id
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
void build_cut_full(WorkerMasterPtr & master, AllCutPackage const & all_package, Str2Int & problem_to_id, SlaveCutId & slave_cut_id, AllCutStorage & all_cuts_storage, DynamicAggregateCuts & dynamic_aggregate_cuts, BendersData & data, BendersOptions & options) {
	check_status(all_package, data);
	if (!options.AGGREGATION && !options.RAND_AGGREGATION) {
		sort_cut_slave(all_package, master, problem_to_id, all_cuts_storage, data, options, slave_cut_id);
	}
	else if (options.AGGREGATION) {
		sort_cut_slave_aggregate(all_package, master, problem_to_id, all_cuts_storage, data, options);
	}
	else if (options.RAND_AGGREGATION) {
		add_random_cuts(master, all_package, problem_to_id, options, data);
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
void update_active_cuts(WorkerMasterPtr & master, ActiveCutStorage & active_cuts, SlaveCutId & cut_id, int const it) {
	DblVector dual;
	master->get_dual_values(dual);
	for (auto & kvp : cut_id) {
		for (int i(0); i < kvp.second.size(); i++) {
			active_cuts.push_back(std::make_tuple(it, kvp.first, i + 1, (dual[kvp.first[i]] != 0)));
			//	}
			//}
		}
	}
}

void compute_x_cut(BendersOptions const& options, BendersData& data) {
	// initialisation
	if (data.it == 1) {
		data.x_stab = data.x0;
		data.x_cut = data.x0;
	}
	else {
		for (auto const& kvp : data.x0) {
			data.x_cut[kvp.first] = data.stab_value * data.x0[kvp.first] +
				(1 - data.stab_value) * data.x_stab[kvp.first];
		}
	}

	data.ub = 0;
}

void update_in_out_stabilisation(BendersData& data) {

}

void compute_ub(WorkerMasterPtr& master, BendersData& data, BendersOptions const& options) {
	// Taking the obj function of master prob to compute c.x_cut
	DblVector obj;
	int n_cols = master->get_ncols();
	obj.resize(n_cols, -1);
	master->get_obj(obj, 0, n_cols - 1);

	int col_id(0);
	for (auto const& kvp : data.x_cut) {
		col_id = master->_name_to_id[kvp.first];
		data.ub += kvp.second * obj[col_id];
	}
}