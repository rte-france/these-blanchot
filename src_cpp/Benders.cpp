#include "Benders.h"


Benders::~Benders() {
}

Benders::Benders(problem_names const & problem_list) {
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
		stream << std::endl;
	}
}

void Benders::print_log(std::ostream&stream, int it, int maxsimplexiter, int minsimplexiter, int deleted_cut)const {
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
		stream << std::setw(15) << minsimplexiter;
		stream << std::setw(15) << maxsimplexiter;
	}

	if (_options.LOG_LEVEL > 2) {
		stream << std::setw(15) << deleted_cut;
		stream << std::endl;
	}
}

bool Benders::stopping_criterion(int it) {
		return((it > _options.MAX_ITERATIONS)||((_lb + _options.GAP >= _best_ub)));
}

void Benders::run() {
	WorkerMaster & master(*_master);
	_lb = -1e20;
	_ub = +1e20;
	_best_ub = +1e20;

	bool stop = false;
	int it(0);
	init_log(std::cout);
	//master.write(it);
	double alpha(0);
	double slave_cost(0);
	double invest_cost(0);
	Point bestx;
	int nslaves = (int)_slaves.size();
	double dnslaves = (double)_slaves.size();

	for (auto const & kvp : _id_to_problem) {
		_all_cuts_storage[kvp.second] = SlaveCutStorage();
	}

	while (!stop) {

		++it;
		master.solve();

		PointPtr x0(new Point);
		Point s;
		int deleted_cut(0);
		int maxsimplexiter(0);
		int minsimplexiter(1000);
		master.get(*x0, alpha); /*Get the optimal variables of the Master Problem*/
		master.get_value(_lb); /*Get the optimal value of the Master Problem*/
		invest_cost = _lb - alpha;
		_ub = invest_cost;

		for (auto const & kvp : _id_to_problem) {

			int i_slave(kvp.first);
			std::string const & name_slave(kvp.second);

			WorkerSlave & slave(*_slaves[i_slave]);
			slave.fix_to(*x0);
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

			SlaveCutTrimmer trimmercut(handler, x0);
			if (true) {

			}

			if (_all_cuts_storage[name_slave].find(trimmercut) != _all_cuts_storage[name_slave].end())
			{
				deleted_cut++;
			}
			else {
				master.add_cut_slave(i_slave, handler->get_subgradient(), *x0, handler->get_dbl(SLAVE_COST));
				_all_cuts_storage.find(name_slave)->second.insert(trimmercut);
			}

			if (maxsimplexiter < handler->get_int(SIMPLEXITER)) {
				maxsimplexiter = handler->get_int(SIMPLEXITER);
			}
			else if (minsimplexiter > handler->get_int(SIMPLEXITER)) {
				minsimplexiter = handler->get_int(SIMPLEXITER);
			}

		}

		if (_best_ub > _ub) {
			_best_ub = _ub;
			bestx = *x0;
		}

		//master.write(it);

		print_log(std::cout, it, maxsimplexiter, minsimplexiter, deleted_cut);

		stop = stopping_criterion(it);
	}
	std::cout << "Investment solution" << std::endl;
	for (auto const & kvp : bestx) {
		std::cout << std::setw(50) << std::left << kvp.first;
		std::cout << " = ";
		std::cout << std::setw(20) << std::scientific << std::setprecision(10) << kvp.second;
		std::cout << std::endl;
	}
}