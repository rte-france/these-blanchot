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
		stream << std::setw(20) << data.time_it.elapsed();
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
		stream << std::setw(20) << "TIME";
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
	data.time_it.restart();
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
		WorkerSlavePtr & ptr(kvp.second);
		IntVector intParam(SlaveCutInt::MAXINT);
		DblVector dblParam(SlaveCutDbl::MAXDBL);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

		ptr->fix_to(data.x0);
		ptr->solve();
		if (options.BASIS) {
			ptr->get_basis();
		}
		ptr->get_value(handler->get_dbl(SLAVE_COST));
		ptr->get_subgradient(handler->get_subgradient());
		ptr->get_simplex_ite(handler->get_int(SIMPLEXITER));
		slave_cut_package[kvp.first] = *slave_cut_data;
	}
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
}

/*!
*  \brief Initialize set of data used in the loop
*
*  \param data : Benders data 
*/
void init(BendersData & data) {
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
	data.time_it.restart();
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
*  \param data : Benders data 
*
*  \param options : set of parameters
*
*/
void sort_cut_slave(std::vector<SlaveCutPackage> const & all_package, DblVector const & slave_weight_coeff, WorkerMasterPtr & master, std::map<std::string, int> & problem_to_id, std::vector<WorkerMasterDataPtr> & trace, AllCutStorage & all_cuts_storage, BendersData & data, BendersOptions const & options) {
	for (int i(0); i < all_package.size(); i++) {
		for (auto & itmap : all_package[i]) {
			SlaveCutDataPtr slave_cut_data(new SlaveCutData(itmap.second));
			SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));
			handler->get_dbl(ALPHA_I) = data.alpha_i[problem_to_id[itmap.first]];
			data.ub += handler->get_dbl(SLAVE_COST)* slave_weight_coeff[problem_to_id[itmap.first]];

			SlaveCutTrimmer cut(handler, data.x0);
			if (options.DELETE_CUT && !(all_cuts_storage[itmap.first].find(cut) == all_cuts_storage[itmap.first].end())) {
				data.deletedcut++;
			}
			else {
				master->add_cut_slave(problem_to_id[itmap.first], handler->get_subgradient(), data.x0, handler->get_dbl(SLAVE_COST));
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
*/
void print_csv(std::vector<WorkerMasterDataPtr> & trace, std::map<std::string, int> & problem_to_id, BendersData const & data, BendersOptions const & options) {
	 std::string output(options.OUTPUTROOT + PATH_SEPARATOR + "benders_output.csv");
	 if (options.AGGREGATION) {
		 output = (options.OUTPUTROOT + PATH_SEPARATOR + "benders_output_aggregate.csv");
	 }
	 std::ofstream file(output, std::ios::out | std::ios::trunc);
	 if (file)
	 {
		 file << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;deletedcut;time" << std::endl;
		 Point xopt;
		 int nite;
		 nite = trace.size();
		 xopt = trace[nite - 1]->get_point();
		 for (int i(0); i < nite; i++) {
			 file << i + 1 << ";";
			 print_master_csv(file, trace[i], xopt, options.MASTER_NAME, data.nslaves);
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

void print_master_csv(std::ostream&stream, WorkerMasterDataPtr & trace, Point const & xopt, std::string const & name, int const nslaves) {
	stream << "Master" << ";";
	stream << name << ";";
	stream << nslaves << ";";
	stream << trace->get_ub() << ";";
	stream << trace->get_lb() << ";";
	stream << trace->get_bestub() << ";";
	stream << norm_point(xopt, trace->get_point()) << ";";
	stream << trace->get_deletedcut() << std::endl;
}

void print_cut_csv(std::ostream&stream, SlaveCutDataHandler const & handler, std::string const & name, int const islaves) {
	stream << "Slave" << ";";
	stream << name << ";";
	stream << islaves << ";";
	stream << handler.get_dbl(SLAVE_COST) << ";";
	stream << handler.get_dbl(ALPHA_I) << ";";
	stream << ";";
	stream << handler.get_int(SIMPLEXITER) << ";";
	stream << std::endl;
}