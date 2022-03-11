#include "BendersFunctions.h"


/*!
*  \brief Initialize set of data used in the loop
*
*  \param data : Benders data
*/
void init(BendersData & data, BendersOptions const& options) {
	//std::srand(time(NULL));
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
	data.stab_value = options.STEP_SIZE;
	
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

	data.step_size = options.STEP_SIZE;

	data.nocutmaster = 0;
	data.misprices = 0;
	data.nul_simplex_cnt = 0;
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
	else if (options.ALGORITHM == "LEVEL") {
		init_log_level(stream, log_level);
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
*  \brief Initialize Benders log for BASE algorithm
*
*  Method to initialize Benders log by printing each column title
*
*  \param stream : output to print log
*
* \param log_level : level of log precision (from 1 to 3)
*/
void init_log_level(std::ostream& stream, int const log_level) {
	stream << std::setw(10) << "ITE";
	stream << std::setw(20) << "LB";
	stream << std::setw(20) << "LEV";
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
		stream << std::setw(15) << "TOTALTIME";
		stream << std::setw(15) << "TIMEMASTER";
		stream << std::setw(15) << "TIMESLAVES";
		stream << std::setw(15) << "TIMEOTHER";
		stream << std::setw(15) << "NBR_NOCUT";
		stream << std::setw(15) << "STEP_SIZE";
	}
	stream << std::endl;
}

void reset_iteration_data(BendersData& data, BendersOptions const& options)
{
	data.deletedcut = 0;
	data.maxsimplexiter = 0;
	data.minsimplexiter = std::numeric_limits<int>::max();
	data.ub = 0;
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
void print_log(std::ostream&stream, BendersData const & data, int const log_level, 
	BendersOptions const & options) {
	if (options.ALGORITHM == "BASE") {
		print_log_base(stream, data, log_level);
	}
	else if (options.ALGORITHM == "IN-OUT") {
		print_log_inout(stream, data, log_level);
	}
	else if (options.ALGORITHM == "ENHANCED_MULTICUT") {
		print_log_enhanced_multicut(stream, data, log_level);
	}
	else if (options.ALGORITHM == "LEVEL") {
		print_log_level(stream, data, log_level);
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

	stream << std::setw(15) << std::scientific << std::setprecision(2) << (data.best_ub - data.lb) / data.best_ub;

	if (log_level > 1) {
		stream << std::setw(15) << data.minsimplexiter;
		stream << std::setw(15) << data.maxsimplexiter;
	}

	if (log_level > 2) {
		stream << std::setw(15) << data.deletedcut;
		stream << std::setw(15) << std::setprecision(2) << data.time_master;
		stream << std::setw(15) << std::setprecision(2) << data.time_slaves;
	}
	stream << std::endl;
}


/*!
*  \brief Print iteration log for LEVEL algorithm
*
*  Method to print the log of an iteration
*
*  \param stream : output to print log
*
* \param data : data to print
*
* \param log_level : level of log precision (from 1 to 3)
*/
void print_log_level(std::ostream& stream, BendersData const& data, int const log_level) {
	stream << std::setw(10) << data.it;
	if (data.lb == -1e20)
		stream << std::setw(20) << "-INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << data.lb;

	stream << std::setw(20) << std::scientific << std::setprecision(10) << data.level;

	if (data.ub == +1e20)
		stream << std::setw(20) << "+INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << data.ub;
	if (data.best_ub == +1e20)
		stream << std::setw(20) << "+INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << data.best_ub;

	stream << std::setw(15) << std::scientific << std::setprecision(2) << data.best_ub - data.lb;
	stream << std::setw(15) << std::scientific << std::setprecision(2) << (data.best_ub - data.lb) / data.best_ub;

	if (log_level > 1) {
		stream << std::setw(15) << data.minsimplexiter;
		stream << std::setw(15) << data.maxsimplexiter;
	}

	if (log_level > 2) {
		stream << std::setw(15) << data.deletedcut;
		stream << std::setw(15) << std::setprecision(2) << data.time_master;
		stream << std::setw(15) << std::setprecision(2) << data.time_slaves;
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
	stream << std::setw(15) << std::scientific << std::setprecision(2) << (data.best_ub - data.lb) / data.best_ub;

	if (log_level > 1) {
		stream << std::setw(15) << data.minsimplexiter;
		stream << std::setw(15) << data.maxsimplexiter;
	}

	if (log_level > 2) {
		stream << std::setw(15) << data.deletedcut;
		stream << std::setw(15) << std::setprecision(2) << data.time_master;
		stream << std::setw(15) << std::setprecision(2) << data.time_slaves;
		stream << std::setw(15) << std::setprecision(2) << data.stab_value;
		stream << std::setw(15) << std::setprecision(2) << data.nocutmaster;
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
		stream << std::setw(15) << std::setprecision(2) << data.time_total;
		stream << std::setw(15) << std::setprecision(2) << data.time_master;
		stream << std::setw(15) << std::setprecision(2) << data.time_slaves;
		stream << std::setw(15) << std::setprecision(2) << data.time_other;
		stream << std::setw(15) << data.n_slaves_no_cut;
		stream << std::setw(15) << data.step_size;
		stream << std::setw(15) << data.batch_size;
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
void print_cut_csv(std::ostream&stream, SlaveCutDataHandler const & handler, 
	std::string const & name, int const islaves) {
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
void print_solution(std::ostream&stream, Point const & point, bool const filter_non_zero, int status, bool printsol) {
	if (status == OPTIMAL) {
		if (printsol) {
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
		else {
			stream << std::endl;
			stream << "****************************************************" << std::endl;
			stream << "               OPTIMAL SOLUTION FOUND               " << std::endl;
			stream << "****************************************************" << std::endl;
		}
	}
	else if(status == INFEASIBLE){
		stream << std::endl;
		stream << "****************************************************" << std::endl;
		stream << "               THE PROBLEM IS INFEASIBLE			  " << std::endl;
		stream << "****************************************************" << std::endl;
	}
	else if (status == UNBOUNDED) {
		stream << std::endl;
		stream << "****************************************************" << std::endl;
		stream << "               THE PROBLEM IS UNBOUNDED			  " << std::endl;
		stream << "****************************************************" << std::endl;
	}
	else {
		stream << std::endl;
		stream << "****************************************************" << std::endl;
		stream << "  ERROR : PROBLEM STATUS IS UNKNOWN			      " << std::endl;
		stream << "****************************************************" << std::endl;
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
	if (options.ALGORITHM == "ENHANCED_MULTICUT") {
		return(
			((options.MAX_ITERATIONS != -1) && (data.it > options.MAX_ITERATIONS)) ||
			(data.global_prb_status != 0) ||
			(options.TIME_LIMIT > 0 && data.total_time.elapsed() > options.TIME_LIMIT) ||
			//(data.n_slaves_no_cut == data.nslaves && data.lb + options.GAP >= data.ub) ||
			(data.n_slaves_no_cut == data.nslaves) ||
			(data.early_termination)
			);
	}else {
		bool gap_ok = false;
		if (options.GAP_TYPE == "ABSOLUTE") {
			gap_ok = (data.lb + options.GAP >= data.best_ub);
		}
		else {
			gap_ok = ( (data.best_ub - data.lb) <= options.GAP * data.best_ub);
		}
		return(
			((options.MAX_ITERATIONS != -1) && (data.it > options.MAX_ITERATIONS)) ||
			(data.global_prb_status != 0) ||
			gap_ok ||
			(options.TIME_LIMIT > 0 && data.total_time.elapsed() > options.TIME_LIMIT)
			);
	}
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
	
	data.alpha_i.resize(data.nslaves);

	master->solve_integer(data.master_status, options, 1, "master_");
	
	master->get(data.x0, data.alpha, data.alpha_i); /*Get the optimal variables of the Master Problem*/
	master->get_value(data.lb); /*Get the optimal value of the Master Problem*/

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
int get_slave_cut(SlaveCutPackage & slave_cut_package, SlavesMapPtr & map_slaves, 
	BendersOptions const & options, BendersData & data) {
	
	// Store the status of a non  optimal slave, 0 if all opitmals
	int slaves_worth_status = 0;

	for (auto & kvp : map_slaves) {
		Timer timer_slave;
		WorkerSlavePtr & ptr(kvp.second);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

		ptr->fix_to(data.x_cut);
		ptr->solve(handler->get_int(LPSTATUS), options, data.nslaves, kvp.first);
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
	StrVector const & slaves, BendersOptions const & options, BendersData const& data, 
	Str2Int & problem_to_id) {
	
	// Store the status of a non  optimal slave, 0 if all opitmals
	int slaves_worth_status = 0;
	
	// Computing the maximum number of problems we can solve
	//std::cout << " /!\ EN MPI, IL FAUT CALCULER LE NOMBRE DE SLAVES SUR LA MACHINE " << std::endl;
	int local_nslaves = map_slaves.size();

	int n_slaves_to_solve = std::min(data.batch_size, local_nslaves - data.n_slaves_no_cut);

	int counter = 0;

	int begin = data.n_slaves_no_cut;

	//for (int i(0); i < data.nrandom; i++){
	for (int i(begin); i < begin + n_slaves_to_solve; i++) {
		Timer timer_slave;
		std::string const name_slave(slaves[data.indices[i]]);

		WorkerSlavePtr & ptr(map_slaves[name_slave]);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

		ptr->fix_to(data.x_cut);
		ptr->solve(handler->get_int(LPSTATUS), options, data.nslaves, name_slave);

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
*  Method to add cut from a slave to the Master Problem and store this cut in a map 
*  linking each slave to its set of cuts.
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
void sort_cut_slave(AllCutPackage const & all_package, WorkerMasterPtr & master, 
	Str2Int & problem_to_id, AllCutStorage & all_cuts_storage, BendersData & data, 
	BendersOptions const & options, SlaveCutId & slave_cut_id) {
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & itmap : all_package[i]) {

			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			handler->get_dbl(ALPHA_I) = data.alpha_i[problem_to_id[itmap.first]];
			data.ub += (1.0/data.nslaves) * handler->get_dbl(SLAVE_COST);
			SlaveCutTrimmer cut(handler, data.x_cut);

			if (options.DELETE_CUT && !(all_cuts_storage[itmap.first].find(cut) == 
				all_cuts_storage[itmap.first].end())) {
				data.deletedcut++;
			}
			else {
				if ( 1==1 || has_cut_master(master, data, options, problem_to_id[itmap.first], 
					handler->get_dbl(SLAVE_COST), handler->get_subgradient()) ) 
				{
					master->add_cut_slave(problem_to_id[itmap.first], handler->get_subgradient(), 
						data.x_cut, handler->get_dbl(SLAVE_COST));
					//all_cuts_storage[itmap.first].insert(cut);
				}
				else {
					data.nocutmaster	+= 1;
					data.misprices		+= 1;
					master->add_cut_slave(problem_to_id[itmap.first], handler->get_subgradient(), 
						data.x_cut, handler->get_dbl(SLAVE_COST));
					//all_cuts_storage[itmap.first].insert(cut);
				}
			}

			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);
		}
	}
}

/*!
*  \brief Add aggregated cut to Master Problem and store it in a set
*
*  Method to add aggregated cut from slaves to Master Problem and store it in a map 
*  linking each slave to its set of non-aggregated cut
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
void sort_cut_slave_aggregate(AllCutPackage const & all_package, WorkerMasterPtr & master, 
	Str2Int & problem_to_id, AllCutStorage & all_cuts_storage, BendersData & data, 
	BendersOptions const & options) {
	Point s;
	double rhs(0);
	for (int i(0); i < all_package.size(); i++) {
		for (auto const & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			data.ub += (1.0/data.nslaves) * handler->get_dbl(SLAVE_COST);
			rhs += handler->get_dbl(SLAVE_COST);
			for (auto const & var : data.x_cut) {
				s[var.first] += handler->get_subgradient()[var.first];
			}
			SlaveCutTrimmer cut(handler, data.x_cut);
			if (options.DELETE_CUT && !(all_cuts_storage[itmap.first].find(cut) == 
				all_cuts_storage[itmap.first].end())) {
				data.deletedcut++;
			}
			//all_cuts_storage.find(itmap.first)->second.insert(cut);

			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);
		}
	}

	master->add_cut(s, data.x_cut, rhs);
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
void add_random_cuts(WorkerMasterPtr & master, AllCutPackage const & all_package, 
	Str2Int & problem_to_id, BendersOptions & options, BendersData & data) {
	int nboundslaves(0);
	
	// Counter of number of subproblems which were not really cut this ite
	int optcounter = 0;
	int total_counter = 0;

	data.stay_in_x_cut = false;
	double gap = compute_gap(options, data);

	for (int i(0); i < all_package.size(); i++) {
		for (auto const & kvp : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(kvp.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));
			master->add_cut_slave(problem_to_id[kvp.first], handler->get_subgradient(), 
				data.x_cut, handler->get_dbl(SLAVE_COST));
			handler->get_dbl(ALPHA_I) = data.alpha_i[problem_to_id[kvp.first]];
			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);

			// Check if the cut has really cut or not
			if (has_cut_master(master, data, options, problem_to_id[kvp.first],
				handler->get_dbl(SLAVE_COST), handler->get_subgradient())) {
				data.has_cut = true;
			}
			else {
				data.misprices += 1;
				data.nocutmaster += 1;
			}

			// Check local optimality
			data.espilon_s = std::min( ( gap - data.epsilon_x) , data.remaining_gap);
			
			if ( (handler->get_dbl(SLAVE_COST) - handler->get_dbl(ALPHA_I) < data.espilon_s) ) {
				optcounter += 1;
			}

			total_counter += 1;
			data.n_slaves_solved += 1;

			data.remaining_gap -= std::max(handler->get_dbl(SLAVE_COST) 
				- handler->get_dbl(ALPHA_I), 0.0);
		}
	}

	data.final_gap = ( gap - data.epsilon_x) - data.remaining_gap;
	udpate_number_nocut(options, data, optcounter, total_counter);
}


/*!
*  \brief Add an aggregation of the random cuts in master problem
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
void add_aggregated_random_cuts(WorkerMasterPtr& master, AllCutPackage const& all_package, 
	Str2Int& problem_to_id, BendersOptions& options, BendersData& data)
{
	int nboundslaves(0);

	// Counter of number of subproblems which were not really cut this ite
	int optcounter = 0;
	int total_counter = 0;

	data.stay_in_x_cut = false;
	double gap = compute_gap(options, data);

	Point s;
	double rhs(0);
	IntVector ids;
	ids.clear();
	s.clear();

	double local_ub			= 0;
	double local_epigraph	= 0;


	for (int i(0); i < all_package.size(); i++) {

		local_ub		= 0;
		local_epigraph	= 0;

		for (auto const& kvp : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(kvp.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));

			//std::cout << kvp.first << std::endl;

			handler->get_dbl(ALPHA_I) += data.alpha_i[problem_to_id[kvp.first]];
			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);

			ids.push_back(problem_to_id[kvp.first]);
			
			rhs += handler->get_dbl(SLAVE_COST);
			data.ub += (1.0 / data.nslaves) * handler->get_dbl(SLAVE_COST);

			for (auto const& var : data.x_cut) {
				s[var.first] += handler->get_subgradient()[var.first];
			}

			// Check if the cut has really cut or not
			if (has_cut_master(master, data, options, problem_to_id[kvp.first],
				handler->get_dbl(SLAVE_COST), handler->get_subgradient())) {
				data.has_cut = true;
			}
			else {
				data.misprices += 1;
				data.nocutmaster += 1;
			}


			local_ub		+= handler->get_dbl(SLAVE_COST);
			local_epigraph	+= handler->get_dbl(ALPHA_I);

			data.espilon_s = data.remaining_gap;

			total_counter += 1;
			data.n_slaves_solved += 1;

		}

		// Check local optimality
		data.espilon_s = data.remaining_gap;

		if (std::max(local_ub - local_epigraph, 0.0) < data.espilon_s) {
			optcounter += total_counter;
		}

		data.remaining_gap -= std::max(local_ub - local_epigraph, 0.0);
	}

	master->add_agregated_cut_slaves(ids, s, data.x_cut, rhs);
	udpate_number_nocut(options, data, optcounter, total_counter);
}


void add_aggregated_cuts_trukanov(WorkerMasterPtr& master, AllCutPackage const& all_package,
	Str2Int& problem_to_id, BendersOptions& options, BendersData& data)
{
	int n_batches = data.batches.size();

	std::vector<Point> s(n_batches);
	std::vector<double> rhs(n_batches, 0);
	std::vector<IntVector> ids(n_batches);
	//ids.clear();
	//s.clear();
	for (auto id : ids) {
		id.clear();
	}
	for (auto subgrad : s) {
		subgrad.clear();
	}

	int current_batch = 0;

	for (int i(0); i < all_package.size(); i++) {
		for (auto const& kvp : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(kvp.second));
			SlaveCutDataHandlerPtr const handler(new SlaveCutDataHandler(slave_cut_data));

			current_batch = data.name_to_batch[kvp.first];
			//std::cout << kvp.first << std::endl;

			handler->get_dbl(ALPHA_I) += data.alpha_i[problem_to_id[kvp.first]];
			bound_simplex_iter(handler->get_int(SIMPLEXITER), data);

			ids[current_batch].push_back(problem_to_id[kvp.first]);

			rhs[current_batch] += handler->get_dbl(SLAVE_COST);
			data.ub += (1.0 / data.nslaves) * handler->get_dbl(SLAVE_COST);

			for (auto const& var : data.x_cut) {
				s[current_batch][var.first] += handler->get_subgradient()[var.first];
			}
		}
	}
	for (int i = 0; i < n_batches; i++) {
		/*for (auto id : ids[i]) {
			std::cout << id << "  ";
		}
		std::cout << std::endl;*/
		master->add_agregated_cut_slaves(ids[i], s[i], data.x_cut, rhs[i]);
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
void build_cut_full(WorkerMasterPtr & master, AllCutPackage const & all_package, 
	Str2Int & problem_to_id, SlaveCutId & slave_cut_id, AllCutStorage & all_cuts_storage, 
	BendersData & data, BendersOptions & options) {
	check_status(all_package, data);

	if (options.ALGORITHM == "BASE" || options.ALGORITHM == "IN-OUT") {
		if (options.AGGREGATION) {
			// Local aggregation : trukhanov
			if (options.AGGREGATION_LEVEL == 1) {
				add_aggregated_cuts_trukanov(master, all_package, problem_to_id, options, data);
			}
			// Pure monocut
			else if (options.AGGREGATION_LEVEL = 2) {
				sort_cut_slave_aggregate(all_package, master, problem_to_id, all_cuts_storage, data, options);
			}
			else {
				std::cout << "Unknown aggregation level " << options.AGGREGATION_LEVEL << std::endl;
				std::exit(1);
			}
		}
		// Pure multicut
		else {
			sort_cut_slave(all_package, master, problem_to_id, all_cuts_storage, data, options, slave_cut_id);
		}
	}
	else if (options.ALGORITHM == "ENHANCED_MULTICUT") {
		if (options.AGGREGATION) {
			add_aggregated_random_cuts(master, all_package, problem_to_id, options, data);
		}
		else {
			add_random_cuts(master, all_package, problem_to_id, options, data);
		}
	}
	else if (options.ALGORITHM == "LEVEL") {
		sort_cut_slave_aggregate(all_package, master, problem_to_id, all_cuts_storage, data, options);
	}
}

/*!
*  \brief Compute the point in which the subproblems will be solveds
*
*  \param options : algorithm saying in which way the separation point is computed
*
*  \param data : data of Benders resolution
*/
void compute_x_cut(BendersOptions const& options, BendersData& data) {
	
	if (options.ALGORITHM == "BASE" || options.ALGORITHM == "LEVEL") {
		data.x_stab = data.x0;
		data.x_cut = data.x0;
	}
	else if (options.ALGORITHM == "IN-OUT") {
		if (data.it <= 1 && !options.INIT_MEAN_VALUE_SOLUTION) {
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
		if (data.it == 0 ) {
			data.x_stab = data.x0;
			data.x_cut = data.x0;
			data.x_mem = data.x0;
		}
		else {
			data.x_stab = data.x_cut;
			if (options.MEMORY_TYPE == "SOLUTION") {
				for (auto const& kvp : data.x_mem) {
					data.x_mem[kvp.first] = (1.0 - options.BETA) * data.x0[kvp.first] +
						options.BETA * data.x_mem[kvp.first];

					data.x_cut[kvp.first] = data.step_size * data.x_mem[kvp.first] +
						(1.0 - data.step_size) * data.x_stab[kvp.first];
				}
			}
			else if (options.MEMORY_TYPE == "WITHOUT") {
				for (auto const& kvp : data.x0) {
					data.x_cut[kvp.first] = data.step_size * data.x0[kvp.first] +
						(1.0 - data.step_size) * data.x_stab[kvp.first];
				}
			}
			else {
				std::cout << "WRONG MEMORY TYPE" << std::endl;
				std::exit(0);
			}
			
		}
	}
	else {
		std::cout << "ALGORITHM " << options.ALGORITHM << " UNKNOWN" << std::endl;
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
void update_in_out_stabilisation(WorkerMasterPtr & master, BendersData& data, BendersOptions const& options) {
	if (data.ub < data.best_ub) {
		data.x_stab = data.x_cut;
		if (options.ALPHA_STRAT == "DYNAMIQUE") {
			data.stab_value = std::min(1.0, 1.2 * data.stab_value);
		}
	}
	else {
		if (options.ALPHA_STRAT == "DYNAMIQUE") {
			data.stab_value = std::max(0.1, 0.8 * data.stab_value);
		}
	}
}

/*!
*  \brief Compute the value of the separation point
*
*  Compute the actual value of the separation point by taking the sum of the valu 
*  of all the subproblem and adding the first stage variables value
*
*  \param master : pointer to master problem
*
*  \param data : data of Benders
*/
void compute_ub(WorkerMasterPtr& master, BendersData& data) {
	// Taking the obj function of master prob to compute c.x_cut
	/*DblVector obj;
	int n_cols = master->get_ncols();
	obj.resize(n_cols, -1);
	master->get_obj(obj, 0, n_cols - 1);

	int col_id(0);
	for (auto const& kvp : data.x_cut) {
		col_id = master->_name_to_id[kvp.first];
		data.ub += kvp.second * obj[col_id];
	}*/

	data.invest_separation_cost = 0;
	int col_id = 0;
	for (auto const& kvp : data.x_cut) {
		col_id = master->_name_to_id[kvp.first];
		data.invest_separation_cost += kvp.second * master->_initial_obj[col_id];
	}
	data.ub += data.invest_separation_cost;
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
		std::rotate(data.indices.begin(), data.indices.begin() + data.n_slaves_no_cut + options.BATCH_SIZE, data.indices.end());
	}
	else if (options.SORTING_METHOD == "RANDOM"){
		std::random_shuffle(data.indices.begin(), data.indices.end());
	}
	else {
		std::cout << "SORTING METHOD UNKNOWN. Please check README.txt to see available methods." << std::endl;
		std::exit(0);
	}
}

void compute_separation_point_cost(WorkerMasterPtr& master, BendersData& data, BendersOptions const& options)
{
	data.invest_separation_cost = 0;
	// Taking the obj function of master prob to compute c.x_cut
	DblVector obj;
	int n_cols = master->get_ncols();
	obj.resize(n_cols, -1);
	master->get_obj(obj, 0, n_cols - 1);

	int col_id(0);
	for (auto const& kvp : data.x_cut) {
		col_id = master->_name_to_id[kvp.first];
		data.invest_separation_cost += kvp.second * obj[col_id];
	}
}

bool has_cut_master(WorkerMasterPtr& master, BendersData& data, BendersOptions const& options, 
	int id, double val, Point subgrad)
{
	if (data.alpha == options.THETA_LB) {
		return true;
	}
	
	double delta_cut = val - data.alpha_i[id];
	for (auto const& kvp : subgrad) {
		delta_cut += kvp.second * (data.x0[kvp.first] - data.x_cut[kvp.first]);
	}
	return delta_cut >= options.CUT_MASTER_TOL;
}

void compute_epsilon_x(WorkerMasterPtr& master, BendersOptions const& options, BendersData& data)
{
	data.epsilon_x = 0;
	// Taking the obj function of master prob to compute c.x_cut
	DblVector obj;
	int n_cols = master->get_ncols();
	obj.resize(n_cols, -1);
	master->get_obj(obj, 0, n_cols - 1);

	int col_id(0);
	for (auto const& kvp : data.x_cut) {
		col_id = master->_name_to_id[kvp.first];
		data.epsilon_x += (kvp.second - data.x0[kvp.first]) * obj[col_id];
	}
}

void del_last_rows(WorkerMasterPtr& master, BendersOptions const& options, BendersData& data)
{
	master->delete_constraint(data.nslaves);
}

void numerical_emphasis(WorkerMasterPtr& master, BendersOptions const& options)
{
	master->_solver->numerical_emphasis(options.NUMERICAL_EMPHASIS);
}

double compute_gap(BendersOptions const& options, BendersData& data)
{
	double gap = 0.0;
	if (options.GAP_TYPE == "RELATIVE") {
		gap = data.lb * options.GAP;
	}
	else {
		gap = options.GAP;
	}

	if (data.n_slaves_no_cut == 0) {
		data.remaining_gap = (gap - data.epsilon_x);
	}

	return gap;
}

void udpate_number_nocut(BendersOptions const& options, BendersData& data, 
	int n_nocut, int n_slaves_solved)
{
	if (n_nocut == n_slaves_solved) {
		data.n_slaves_no_cut += n_nocut;
		if (data.n_slaves_no_cut < data.nslaves) {
			data.stay_in_x_cut = true;
		}
	}
	else {
		data.n_slaves_no_cut = 0;
	}
}
