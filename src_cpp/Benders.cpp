#include "Benders.h"


Benders::~Benders() {
}

Benders::Benders(problem_names const & problem_list, BendersOptions const & options) {
	if (!problem_list.empty()) {
		int nslaves = static_cast<int>(problem_list.size()) - 1;
		_slaves.reserve(nslaves);

		auto it(problem_list.begin());
		auto end(problem_list.end());
		_master.reset(new WorkerMaster(*it, nslaves));
		int i(0);
		while(++it != end) {
			_id_to_problem[i] = *it; 
			_slaves.push_back(WorkerSlavePtr(new WorkerSlave(*it)));
			i++;
		}
	}
	_options = options;
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
	stream << std::setw(10) << _it;
	if (_lb == -1e20)
		stream << std::setw(20) << "-INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << _lb;
	if (_ub == +1e20)
		stream << std::setw(20) << "+INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << _ub;
	if (_best_ub == +1e20)
		stream << std::setw(20) << "+INF";
	else
		stream << std::setw(20) << std::scientific << std::setprecision(10) << _best_ub;

	if (_options.LOG_LEVEL > 1) {
		stream << std::setw(15) << _minsimplexiter;
		stream << std::setw(15) << _maxsimplexiter;
	}

	if (_options.LOG_LEVEL > 2) {
		stream << std::setw(15) << _deletedcut;
	}
	stream << std::endl;

}

bool Benders::stopping_criterion() {
	_deletedcut = 0;
	_maxsimplexiter = 0;
	_minsimplexiter = 1000;
	return(((_options.MAX_ITERATIONS != -1)&&(_it > _options.MAX_ITERATIONS))||(_lb + _options.GAP >= _best_ub));
}

void Benders::init() {
	_lb = -1e20;
	_ub = +1e20;
	_best_ub = +1e20;
	_stop = false;
	_it = 0;
	_alpha = 0;
	_slave_cost = 0;
	_invest_cost = 0;
	_nslaves = (int)_slaves.size();
	_dnslaves = (double)_slaves.size();
	_deletedcut = 0;
	_maxsimplexiter = 0;
	_minsimplexiter = 1000;

	for (auto const & kvp : _id_to_problem) {
		_all_cuts_storage[kvp.second] = SlaveCutStorage();
	}
}

void Benders::bound_simplex_iter(int simplexiter) {
	if (_maxsimplexiter < simplexiter) {
		_maxsimplexiter = simplexiter;
	}

	if (_minsimplexiter > simplexiter) {
		_minsimplexiter = simplexiter;
	}
}

void Benders::step_1() {
	_master->solve();
	_master->get(_x0, _alpha); /*Get the optimal variables of the Master Problem*/
	_master->get_value(_lb); /*Get the optimal value of the Master Problem*/
	_invest_cost = _lb - _alpha;
	_ub = _invest_cost;
}

void Benders::step_2() {
	WorkerSlaveTracePtr slave_trace_ptr(new WorkerSlaveTrace);

	for (auto const & kvp : _id_to_problem) {

		int i_slave(kvp.first);
		std::string const & name_slave(kvp.second);

		WorkerSlave & slave(*_slaves[i_slave]);
		slave.fix_to(_x0);
		//slave.write(it);

		IntVector intParam(SlaveCutInt::MAXINT);
		DblVector dblParam(SlaveCutDbl::MAXDBL);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

		slave.solve();
		slave.get_value(handler->get_dbl(SLAVE_COST));
		slave.get_subgradient(handler->get_subgradient());
		slave.get_simplex_ite(handler->get_int(SIMPLEXITER));
		_ub += handler->get_dbl(SLAVE_COST);

		SlaveCutTrimmer trimmercut(handler, _x0);

		if (_all_cuts_storage[name_slave].find(trimmercut) != _all_cuts_storage[name_slave].end())
		{
			_deletedcut++;
		}
		else {
			_master->add_cut_slave(i_slave, handler->get_subgradient(), _x0, handler->get_dbl(SLAVE_COST));
			_all_cuts_storage.find(name_slave)->second.insert(trimmercut);
		}

		slave_trace_ptr->_cut_trace.push_back(slave_cut_data);
		bound_simplex_iter(handler->get_int(SIMPLEXITER));

	}
	
	_trace._x0.push_back(PointPtr(new Point(_x0)));
	_trace._slave_trace.push_back(slave_trace_ptr);
}

void Benders::step_2_aggregate() {
	Point s;
	for (auto & var : _x0) {
		s[var.first] = 0;
	}
	double rhs(0);
	for (auto const & kvp : _id_to_problem) {

		int i_slave(kvp.first);
		std::string const & name_slave(kvp.second);

		WorkerSlave & slave(*_slaves[i_slave]);
		slave.fix_to(_x0);
		//slave.write(it);

		IntVector intParam(SlaveCutInt::MAXINT);
		DblVector dblParam(SlaveCutDbl::MAXDBL);
		SlaveCutDataPtr slave_cut_data(new SlaveCutData);
		SlaveCutDataHandlerPtr handler(new SlaveCutDataHandler(slave_cut_data));

		slave.solve();
		slave.get_value(handler->get_dbl(SLAVE_COST));
		slave.get_subgradient(handler->get_subgradient());
		slave.get_simplex_ite(handler->get_int(SIMPLEXITER));
		_ub += handler->get_dbl(SLAVE_COST);
		rhs += handler->get_dbl(SLAVE_COST);

		for (auto & var : s) {
			s[var.first] += handler->get_subgradient()[var.first];
		}

		SlaveCutTrimmer trimmercut(handler, _x0);

		if (_all_cuts_storage[name_slave].find(trimmercut) != _all_cuts_storage[name_slave].end())
		{
			_deletedcut++;
		}
		_master->add_cut(s, _x0, rhs);
		_all_cuts_storage.find(name_slave)->second.insert(trimmercut);

		bound_simplex_iter(handler->get_int(SIMPLEXITER));

	}
}

void Benders::step_3() {
	if (_best_ub > _ub) {
		_best_ub = _ub;
		_bestx = _x0;
	}
}


void Benders::run(std::ostream & stream) {

	WorkerMaster & master(*_master);
	
	init_log(stream);

	init();

	while (!_stop) {

		++_it;

		//Solve Master problem and get the trial values
		step_1();

		//Solve Slaves problem and add cuts to Master problem
		if (_options.AGGREGATION) {
			step_2_aggregate();
		}
		else {
			step_2();
		}

		//Update best upper bound 
		step_3();

		print_log(stream);

		_stop = stopping_criterion();
	}
	
	print_solution(stream);
	std::cout << "There are " << _trace._slave_trace.size() << " cuts stored" << std::endl;
}

void Benders::print_solution(std::ostream&stream)const {
	stream << "Investment solution" << std::endl;
	for (auto const & kvp : _bestx) {
		stream << std::setw(50) << std::left << kvp.first;
		stream << " = ";
		stream << std::setw(20) << std::scientific << std::setprecision(10) << kvp.second;
		stream << std::endl;
	}
}