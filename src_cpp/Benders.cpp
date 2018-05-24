#include "Benders.h"


Benders::~Benders() {
}

Benders::Benders(problem_names const & problem_list, BendersOptions const & options) {
	_options = options;
	if (!problem_list.empty()) {
		_data.nslaves = static_cast<int>(problem_list.size()) - 1;
		_slaves.reserve(_data.nslaves);
		auto it(problem_list.begin());
		auto end(problem_list.end());
		
		int i(0);
		while(++it != end) {
			_id_to_problem[i] = *it; 
			_problem_to_id[*it] = i;
			_slaves.push_back(WorkerSlavePtr(new WorkerSlave(*it)));
			i++;
		}
		init_slave_weight();
		it = problem_list.begin();
		_master.reset(new WorkerMaster(*it, _slave_weight_coeff, _data.nslaves));
	}

}

void Benders::free() {
	_master->free();
	for (auto & ptr : _slaves)
		ptr->free();
}

void Benders::init_log(std::ostream&stream )const {
	stream << std::setw(10) << "ITE";
	stream << std::setw(20) << "LB";
	stream << std::setw(20) << "UB";
	stream << std::setw(20) << "BESTUB";

	if (_options.LOG_LEVEL > 1) {
		stream << std::setw(15) << "MINSIMPLEXIT";
		stream << std::setw(15) << "MAXSIMPLEXIT";
	}

	if (_options.LOG_LEVEL > 2) {
		stream << std::setw(15) << "DELETEDCUT";
	}
	stream << std::endl;
}

void Benders::print_log(std::ostream&stream) const {

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

bool Benders::stopping_criterion() {
	_data.deletedcut = 0;
	_data.maxsimplexiter = 0;
	_data.minsimplexiter = 1000000;
	return(((_options.MAX_ITERATIONS != -1)&&(_data.it > _options.MAX_ITERATIONS))||(_data.lb + _options.GAP >= _data.best_ub));
}

void Benders::init() {
	_data.lb = -1e20;
	_data.ub = +1e20;
	_data.best_ub = +1e20;
	_data.stop = false;
	_data.it = 0;
	_data.alpha = 0;
	_data.slave_cost = 0;
	_data.invest_cost = 0;
	_data.dnslaves = (double)_slaves.size();
	_data.deletedcut = 0;
	_data.maxsimplexiter = 0;
	_data.minsimplexiter = 1000000;
	

	for (auto const & kvp : _id_to_problem) {
		_all_cuts_storage[kvp.second] = SlaveCutStorage();
	}
}

void Benders::bound_simplex_iter(int simplexiter) {
	if (_data.maxsimplexiter < simplexiter) {
		_data.maxsimplexiter = simplexiter;
	}

	if (_data.minsimplexiter > simplexiter) {
		_data.minsimplexiter = simplexiter;
	}
}

void Benders::get_master_value() {
	_data.alpha_i.resize(_data.nslaves);
	_master->solve();
	_master->get(_data.x0, _data.alpha, _data.alpha_i); /*Get the optimal variables of the Master Problem*/
	_master->get_value(_data.lb); /*Get the optimal value of the Master Problem*/
	_data.invest_cost = _data.lb - _data.alpha;
	_data.ub = _data.invest_cost;

	if (_options.TRACE) {
		_trace._master_trace.push_back(WorkerMasterDataPtr(new WorkerMasterData));
	}

}

void Benders::sort_cut(SlaveCutDataHandlerPtr & handler, int i_slave, std::string const & name_slave) {
	SlaveCutTrimmer trimmercut(handler, _data.x0);
	
	if (_all_cuts_storage[name_slave].find(trimmercut) != _all_cuts_storage[name_slave].end())
	{
		_data.deletedcut++;
	}
	else {
		_master->add_cut_slave(i_slave, handler->get_subgradient(), _data.x0, handler->get_dbl(SLAVE_COST));
		_all_cuts_storage.find(name_slave)->second.insert(trimmercut);
	}
}

void Benders::get_slave_cut(int i_slave, std::string const & name_slave, SlaveCutDataHandlerPtr & handler) {
	WorkerSlave & slave(*_slaves[i_slave]);
	slave.fix_to(_data.x0);
	
	slave.solve();
	slave.get_value(handler->get_dbl(SLAVE_COST));
	slave.get_subgradient(handler->get_subgradient());
	slave.get_simplex_ite(handler->get_int(SIMPLEXITER));
	_data.ub += handler->get_dbl(SLAVE_COST)*_slave_weight_coeff[i_slave];

}

void Benders::update_trace() {
	_trace._master_trace[_data.it - 1]->_lb = _data.lb;
	_trace._master_trace[_data.it - 1]->_ub = _data.ub;
	_trace._master_trace[_data.it - 1]->_bestub = _data.best_ub;
	_trace._master_trace[_data.it - 1]->_x0 = PointPtr(new Point(_data.x0));
	_trace._master_trace[_data.it - 1]->_deleted_cut = _data.deletedcut;
}

void Benders::build_cut() {

	for (auto const & kvp : _id_to_problem) {

		int i_slave(kvp.first);
		std::string const & name_slave(kvp.second);

		IntVector intParam(SlaveCutInt::MAXINT);
		DblVector dblParam(SlaveCutDbl::MAXDBL);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

		get_slave_cut(i_slave, name_slave, handler);
		sort_cut(handler, i_slave, name_slave);
		handler->get_dbl(ALPHA_I) = _data.alpha_i[i_slave];

		if (_options.TRACE) {
			_trace._master_trace[_data.it - 1]->_cut_trace[name_slave] = slave_cut_data;
		}
		bound_simplex_iter(handler->get_int(SIMPLEXITER));
	}
}

void Benders::init_slave_weight() {
	_slave_weight_coeff.resize(_data.nslaves);
	if (_options.SLAVE_WEIGHT == "UNIFORM") {
		for (int i(0); i < _data.nslaves; i++) {
			_slave_weight_coeff[i] = 1/static_cast<double>(_data.nslaves);
		}
	}
	else if (_options.SLAVE_WEIGHT == "ONES") {
		for (int i(0); i < _data.nslaves; i++) {
			_slave_weight_coeff[i] = 1;
		}
	}
	else {
		std::ifstream file(_options.SLAVE_WEIGHT);
		if (!file) {
			std::cout << "Cannot open file " << _options.SLAVE_WEIGHT << std::endl;
		}
		std::string line;
		std::size_t found = _id_to_problem.begin()->second.find_last_of(PATH_SEPARATOR);
		std::string root;
		root = _id_to_problem.begin()->second.substr(0, found);
		while (std::getline(file, line))
		{
			std::stringstream buffer(line);
			std::string problem_name;
			buffer >> problem_name;
			problem_name = root + PATH_SEPARATOR + problem_name;
			buffer >> _slave_weight_coeff[_problem_to_id[problem_name]];
		}
	}
}

void Benders::sort_cut_aggregate(SlaveCutDataHandlerPtr & handler, int i_slave, std::string const & name_slave, Point & s, double & rhs) {

	rhs += handler->get_dbl(SLAVE_COST)*_slave_weight_coeff[i_slave];

	for (auto & var : _data.x0) {
		s[var.first] += handler->get_subgradient()[var.first]*_slave_weight_coeff[i_slave];
	}

	SlaveCutTrimmer trimmercut(handler, _data.x0);

	if (_all_cuts_storage[name_slave].find(trimmercut) != _all_cuts_storage[name_slave].end())
	{
		_data.deletedcut++;
	}
	_all_cuts_storage.find(name_slave)->second.insert(trimmercut);
}

void Benders::build_cut_aggregate() {
	Point s;
	double rhs(0);

	for (auto const & kvp : _id_to_problem) {

		int i_slave(kvp.first);
		std::string const & name_slave(kvp.second);

		IntVector intParam(SlaveCutInt::MAXINT);
		DblVector dblParam(SlaveCutDbl::MAXDBL);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

		get_slave_cut(i_slave, name_slave, handler);
		sort_cut_aggregate(handler, i_slave, name_slave, s, rhs);

		bound_simplex_iter(handler->get_int(SIMPLEXITER));

		if (_options.TRACE) {
			_trace._master_trace[_data.it - 1]->_cut_trace[name_slave] = slave_cut_data;
		}
	}
	_master->add_cut(s, _data.x0, rhs);

}

void Benders::update_best_ub() {
	if (_data.best_ub > _data.ub) {
		_data.best_ub = _data.ub;
		_data.bestx = _data.x0;
	}
}


void Benders::run(std::ostream & stream) {

	WorkerMaster & master(*_master);
	
	init_log(stream);
	init();

	master.write(0);
	while (!_data.stop) {

		++_data.it;
		//Solve Master problem and get the trial values
		get_master_value();

		//Solve Slaves problem and add cuts to Master problem
		if (_options.AGGREGATION) {
			build_cut_aggregate();
		}
		else {
			build_cut();
		}

		update_best_ub();

		//Update best upper bound 
		print_log(stream);

		if (_options.TRACE) {
			update_trace();
		}

		_data.stop = stopping_criterion();
	}
	
	print_solution(stream);
	print_csv();

}

void Benders::print_solution(std::ostream&stream)const {
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

void Benders::print_csv() {
	std::string output(_options.ROOTPATH + PATH_SEPARATOR + "benders_output.csv");
	if (_options.AGGREGATION) {
		output = (_options.ROOTPATH + PATH_SEPARATOR + "benders_output_aggregate.csv");
	}
	std::ofstream file(output, std::ios::out | std::ios::trunc);

	if (file)
	{
		file << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;deletedcut" << std::endl;
		Point xopt;
		int nite;
		nite = _trace.get_ite();
		xopt = _trace._master_trace[nite-1]->get_point();
		for (int i(0); i < nite; i++) {
			file << i + 1 << ";";
			file << "Master" << ";";
			file<< "master" << ";";
			file << _slaves.size() << ";";
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
				file << kvp.first.substr(found+1) << ";";
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