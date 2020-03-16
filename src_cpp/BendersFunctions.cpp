#include "BendersFunctions.h"


/*!
*  \brief Initialize set of data used in the loop
*
*  \param data : Benders data
*/
void init(BendersData & data, BendersOptions const& options) {
	std::srand(time(NULL));
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
	
	data.total_time.restart();

	// Sampling order initialization
	data.indices = std::vector<int>(data.nslaves, 0);
	for (unsigned int i = 0; i < data.nslaves; i++) {
		data.indices[i] = i;
	}

	data.n_slaves_no_cut = 0;
	data.has_cut = true;

	data.batch_size = options.BATCH_SIZE;
	data.espilon_s = options.GAP / data.nslaves;
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
void init_log(std::ostream&stream, int const log_level, BendersOptions const& options) {
	if (options.ALGORITHM == "BASE") {
		init_log_base(stream, log_level);
	}
	else if (options.ALGORITHM == "IN-OUT") {
		init_log_inout(stream, log_level);
	}
	else if (options.ALGORITHM == "ENHANCED_MULTICUT") {
		init_log_enhanced_multicut(stream, log_level);
	}
}

/*!
*  \brief Initialize Benders log for BASE algorithm
*
*  Method to initialize Benders log by printing each column title
*
*  \param stream : output to print log
*
* \param log_level : level of log precision (from 1 to 3)
*/
void init_log_base(std::ostream& stream, int const log_level) {
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
*  \brief Initialize Benders log for IN-OUT algorithm
*
*  Method to initialize Benders log by printing each column title
*
*  \param stream : output to print log
*
* \param log_level : level of log precision (from 1 to 3)
*/
void init_log_inout(std::ostream& stream, int const log_level) {
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
		stream << std::setw(15) << "ALPHA";
	}
	stream << std::endl;
}

/*!
*  \brief Initialize Benders log for ENHANCED_MULTICUT algorithm
*
*  Method to initialize Benders log by printing each column title
*
*  \param stream : output to print log
*
* \param log_level : level of log precision (from 1 to 3)
*/
void init_log_enhanced_multicut(std::ostream& stream, int const log_level) {
	stream << std::setw(10) << "ITE";
	stream << std::setw(20) << "LB";

	if (log_level > 1) {
		stream << std::setw(15) << "MINSIMPLEX";
		stream << std::setw(15) << "MAXSIMPLEX";
	}

	if (log_level > 2) {
		stream << std::setw(15) << "DELETEDCUT";
		stream << std::setw(15) << "TIMEMASTER";
		stream << std::setw(15) << "TIMESLAVES";
		stream << std::setw(15) << "NBR_NOCUT";
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
void print_log(std::ostream&stream, BendersData const & data, int const log_level, BendersOptions const & options) {
	if (options.ALGORITHM == "BASE") {
		print_log_base(stream, data, log_level);
	}
	else if (options.ALGORITHM == "IN-OUT") {
		print_log_inout(stream, data, log_level);
	}
	else if (options.ALGORITHM == "ENHANCED_MULTICUT") {
		print_log_enhanced_multicut(stream, data, log_level);
	}
}

/*!
*  \brief Print iteration log for BASE algorithm
*
*  Method to print the log of an iteration
*
*  \param stream : output to print log
*
* \param data : data to print
*
* \param log_level : level of log precision (from 1 to 3)
*/
void print_log_base(std::ostream&stream, BendersData const & data, int const log_level) {
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
*  \brief Print iteration log for IN-OUT algorithm
*
*  Method to print the log of an iteration
*
*  \param stream : output to print log
*
* \param data : data to print
*
* \param log_level : level of log precision (from 1 to 3)
*/
void print_log_inout(std::ostream& stream, BendersData const& data, int const log_level) {
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
		stream << std::setw(15) << std::setprecision(2) << data.stab_value;
	}
	stream << std::endl;
}

/*!
*  \brief Print iteration log for ENHANCED_MULTICUT algorithm
*
*  Method to print the log of an iteration
*
*  \param stream : output to print log
*
* \param data : data to print
*
* \param log_level : level of log precision (from 1 to 3)
*/
void print_log_enhanced_multicut(std::ostream& stream, BendersData const& data, int const log_level) {
	stream << std::setw(10) << data.it;
	if (data.lb == -1e20) {
		stream << std::setw(20) << "-INF";
	}
	else {
		stream << std::setw(20) << std::scientific << std::setprecision(10) << data.lb;
	}
	if (log_level > 1) {
		stream << std::setw(15) << data.minsimplexiter;
		stream << std::setw(15) << data.maxsimplexiter;
	}

	if (log_level > 2) {
		stream << std::setw(15) << data.deletedcut;
		stream << std::setw(15) << std::setprecision(2) << data.timer_master - data.timer_slaves;
		stream << std::setw(15) << std::setprecision(2) << data.timer_slaves;
		stream << std::setw(15) << data.n_slaves_no_cut;
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
*
* \param status : status returned after optimization (0 if optimal)
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
		(data.global_prb_status != 0) ||
		(options.TIME_LIMIT > 0 && data.total_time.elapsed() > options.TIME_LIMIT) ||
		(data.n_slaves_no_cut == data.nslaves)
		);
}

/*!
*  \brief Check if every slave has been solved to optimality
*
*  \param all_package : storage of each slaves status
*
*  \param data : status of resolution of master and subproblems
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
*
*  \param options : option to say if a non optimal problem has to be written before exiting
*/
void get_master_value(WorkerMasterPtr & master, BendersData & data, BendersOptions const & options) {
	Timer timer_master;
	data.alpha_i.resize(data.nslaves);

	master->solve_integer(data.master_status, options, "master_");
	
	master->get(data.x0, data.alpha, data.alpha_i); /*Get the optimal variables of the Master Problem*/
	master->get_value(data.lb); /*Get the optimal value of the Master Problem*/

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
int get_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, BendersOptions const & options, BendersData & data) {
	
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
int get_random_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, 
	StrVector const & slaves, BendersOptions const & options, BendersData const& data, Str2Int & problem_to_id) {
	
	// Store the status of a non  optimal slave, 0 if all opitmals
	int slaves_worth_status = 0;
	
	// Computing the maximum number of problems we can solve
	//std::cout << " /!\ EN MPI, IL FAUT CALCULER LE NOMBRE DE SLAVES SUR LA MACHINE " << std::endl;
	int local_nslaves = map_slaves.size();
	int n_slaves_to_solve = std::min(data.batch_size, local_nslaves - data.n_slaves_no_cut);

	std::cout << "NUMBER SLAVES = " << n_slaves_to_solve << std::endl;

	int counter = 0;

	int begin = data.n_slaves_no_cut;
	//for (int i(0); i < data.nrandom; i++){
	for (int i(begin); i < begin + n_slaves_to_solve; i++) {
		Timer timer_slave;
		std::string const name_slave(slaves[data.indices[i]]);

		std::cout << name_slave << std::endl;

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
*  \param all_cuts_storage : set to store every new cut
*
*  \param slave_cut_id : map linking each cut to its id in the master problem
*
*  \param data : Benders data
*
*  \param options : set of parameters
*
*/
void sort_cut_slave(AllCutPackage const & all_package, WorkerMasterPtr & master, Str2Int & problem_to_id, 
	AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options, SlaveCutId & slave_cut_id) {
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
				master->add_cut_slave(problem_to_id[itmap.first], handler->get_subgradient(), data.x_cut, handler->get_dbl(SLAVE_COST));
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
*  \param options : set of benders options
*
*  \param data : set of benders data
*/
void add_random_cuts(WorkerMasterPtr & master, AllCutPackage const & all_package, Str2Int & problem_to_id, BendersOptions & options, BendersData & data) {
	int nboundslaves(0);
	
	// Counter of number of subproblems which were not really cut this ite
	int counter = 0;
	int total_counter = 0;

	for (int i(0); i < all_package.size(); i++) {
		for (auto const & kvp : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(kvp.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			master->add_cut_slave(problem_to_id[kvp.first], handler->get_subgradient(), data.x0, handler->get_dbl(SLAVE_COST));
			handler->get_dbl(ALPHA_I) = data.alpha_i[problem_to_id[kvp.first]];
			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);

			// Check if the cut has really cut or not
			if (handler->get_dbl(SLAVE_COST) - handler->get_dbl(ALPHA_I) < data.espilon_s) {
				counter += 1;
			}
			total_counter += 1;
		}
	}

	// update number of slaves not cut
	data.n_slaves_no_cut = counter;
	if (counter < total_counter) {
		data.has_cut = true;
		data.n_slaves_no_cut == 0;
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
*  \param slave_cut_id : map linking each slaves to their cuts ids in the master problem
*
*  \param all_cuts_storage : set to store every new cut
*
*  \param options : set of benders options
*
*  \param data : set of benders data
*/
void build_cut_full(WorkerMasterPtr & master, AllCutPackage const & all_package, Str2Int & problem_to_id, SlaveCutId & slave_cut_id, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions & options) {
	check_status(all_package, data);
	if (options.ALGORITHM == "BASE" || options.ALGORITHM == "IN-OUT") {
		if (options.AGGREGATION) {
			sort_cut_slave_aggregate(all_package, master, problem_to_id, all_cuts_storage, data, options);
		}
		else {
			sort_cut_slave(all_package, master, problem_to_id, all_cuts_storage, data, options, slave_cut_id);
		}
	}
	else if (options.ALGORITHM == "ENHANCED_MULTICUT") {
		add_random_cuts(master, all_package, problem_to_id, options, data);
	}

	/*if (!options.AGGREGATION && !options.RAND_AGGREGATION) {
		sort_cut_slave(all_package, master, problem_to_id, all_cuts_storage, data, options, slave_cut_id);
	}
	else if (options.AGGREGATION) {
		sort_cut_slave_aggregate(all_package, master, problem_to_id, all_cuts_storage, data, options);
	}
	else if (options.RAND_AGGREGATION) {
		add_random_cuts(master, all_package, problem_to_id, options, data);
	}*/
}

/*!
*  \brief Compute the point in which the subproblems will be solveds
*
*  \param options : algorithm saying in which way the separation point is computed
*
*  \param data : data of Benders resolution
*/
void compute_x_cut(BendersOptions const& options, BendersData& data) {
	
	if (options.ALGORITHM == "BASE") {
		data.x_stab = data.x0;
		data.x_cut = data.x0;
	}
	else if (options.ALGORITHM == "IN-OUT") {
		if (data.it == 1) {
			data.x_stab = data.x0;
			data.x_cut = data.x0;
		} else {
			for (auto const& kvp : data.x0) {
				data.x_cut[kvp.first] = data.stab_value * data.x0[kvp.first] +
					(1 - data.stab_value) * data.x_stab[kvp.first];
			}
		}
	}
	else if (options.ALGORITHM == "ENHANCED_MULTICUT") {
		data.x_stab = data.x0;
		data.x_cut = data.x0;
	}
	else {
		std::cout << "ALGORITHME " << options.ALGORITHM << " NON RECONNU" << std::endl;
		std::exit(0);
	}

	data.ub = 0;
}

/*!
*  \brief Update in-out stabilization center and value
*
*  Change the stability center if the new point is better than the previous, and change the stabilization value
*
*  \param master : pointer to master problem
*
*  \param data : data of the Benders resolution
*/
void update_in_out_stabilisation(WorkerMasterPtr & master, BendersData& data) {
	if (data.ub < data.best_ub) {
		data.x_stab = data.x_cut;
		data.stab_value = std::min(1.0, 1.2 * data.stab_value);
	}
	else {
		data.stab_value = std::max(0.1, 0.8 * data.stab_value);
	}
}

/*!
*  \brief Compute the value of the separation point
*
*  Compute the actual value of the separation point by taking the sum of the valu of all the subproblem and adding the first stage variables value
*
*  \param master : pointer to master problem
*
*  \param data : data of Benders
*/
void compute_ub(WorkerMasterPtr& master, BendersData& data) {
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

/*!
*  \brief Compute the new order of subproblems
*
*  Compute the new order of subproblems, used in ENHANCED_MULTICUT to know which one is solve first
*
*  \param data		: data of Benders
*  \param options	: options of Benders
*/
void set_slaves_order(BendersData& data, BendersOptions const& options) {
	if (options.SORTING_METHOD == "ORDERED") {
		std::rotate(data.indices.begin(), data.indices.begin() + data.n_slaves_no_cut + 1, data.indices.end());
	}
	else {
		std::cout << "SORTING METHOD UNKNOWN. Please check README.txt to see available methods." << std::endl;
		std::exit(0);
	}
}