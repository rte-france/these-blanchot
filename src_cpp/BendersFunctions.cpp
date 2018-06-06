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

	if (log_level > 1) {
		stream << std::setw(15) << data.minsimplexiter;
		stream << std::setw(15) << data.maxsimplexiter;
	}

	if (log_level > 2) {
		stream << std::setw(15) << data.deletedcut;
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
	if (options.SLAVE_WEIGHT == "UNIFORM") {
		for (int i(0); i < data.nslaves; i++) {
			slave_weight_coeff[i] = 1 / static_cast<double>(data.nslaves);
		}
	}
	else if (options.SLAVE_WEIGHT == "ONES") {
		for (int i(0); i < data.nslaves; i++) {
			slave_weight_coeff[i] = 1;
		}
	}
	else {
		std::string line;
		std::string filename = options.INPUTROOT + PATH_SEPARATOR + options.SLAVE_WEIGHT;
		std::ifstream file(filename);
		if (!file) {
			std::cout << "Cannot open file " << filename << std::endl;
		}
		while (std::getline(file, line))
		{
			std::stringstream buffer(line);
			std::string problem_name;
			buffer >> problem_name;
			problem_name = options.INPUTROOT + PATH_SEPARATOR + problem_name;
			buffer >> slave_weight_coeff[problem_to_id[problem_name]];
			std::cout << problem_name << " : " << problem_to_id[problem_name] << "  :  " << slave_weight_coeff[problem_to_id[problem_name]] << std::endl;
		}
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

	if (log_level > 1) {
		stream << std::setw(15) << "MINSIMPLEXIT";
		stream << std::setw(15) << "MAXSIMPLEXIT";
	}

	if (log_level > 2) {
		stream << std::setw(15) << "DELETEDCUT";
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
void get_master_value(WorkerMasterPtr & master, BendersData & data) {
	data.alpha_i.resize(data.nslaves);
	master->solve();
	master->get(data.x0, data.alpha, data.alpha_i); /*Get the optimal variables of the Master Problem*/
	master->get_value(data.lb); /*Get the optimal value of the Master Problem*/
	data.invest_cost = data.lb - data.alpha;
	data.ub = data.invest_cost;
}


/*!
*  \brief Solve and get optimal variables of a Slave Problem
*
*  Method to solve and get optimal variables of a Slave Problem after fixing trial values
*
*  \param handler : handler to receive cut information
*
*  \param slave : pointer to a Slave Problem
*
*  \param data : data containing trial values
*
*  \param options : set of parameters
*/
void get_slave_cut(SlaveCutDataHandlerPtr & handler, WorkerSlavePtr & slave, BendersOptions const & options, BendersData const & data) {
	slave->fix_to(data.x0);
	slave->solve();
	if (options.BASIS) {
		slave->get_basis();
	}
	slave->get_value(handler->get_dbl(SLAVE_COST));
	slave->get_subgradient(handler->get_subgradient());
	slave->get_simplex_ite(handler->get_int(SIMPLEXITER));
}