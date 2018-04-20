#include "Benders.h"

Benders::~Benders() {

}

Benders::Benders(mps_coupling_list const & mps_coupling_list) {
	if (!mps_coupling_list.empty()) {
		int nslaves = static_cast<int>(mps_coupling_list.size()) - 1;
		_slaves.reserve(nslaves);

		auto it(mps_coupling_list.begin());
		auto end(mps_coupling_list.end());
		_master.reset(new WorkerMaster(it->first, it->second, nslaves));
		while(++it != end) {
			_slaves.push_back(WorkerSlavePtr(new WorkerSlave(it->first, it->second)));
		}
	}
}

void Benders::run() {
	WorkerMaster & master(*_master);
	_lb = -1e20;
	_ub = +1e20;
	_best_ub = +1e20;

	bool stop = false;
	int it(0);


	master.write(it);
	double alpha(0);
	double slave_cost(0);
	double invest_cost(0);
	std::cout << std::setw(10) << "ITE";
	std::cout << std::setw(20) << "LB";
	std::cout << std::setw(20) << "UB";
	std::cout << std::setw(20) << "BESTUB";
	std::cout << std::setw(10) << "SIMPLEXIT";
	std::cout << std::endl;
	Point bestx;
	int simplexiter;
	int nslaves = (int)_slaves.size();
	double dnslaves = (double)_slaves.size();

	while (!stop) {
		++it;
		master.solve();

		Point x0;
		Point s;
		double rhs;
		master.get(x0, alpha); /*Get the optimal variables of the Master Problem*/
		master.get_value(_lb); /*Get the optimal value of the Master Problem*/
		invest_cost = _lb - alpha;
		_ub = invest_cost;
		for (int i_slave(0); i_slave < nslaves; ++i_slave) {
			WorkerSlave & slave(*_slaves[i_slave]);
			slave.fix_to(x0);
			//slave.write(it);

			slave.solve();
			slave.get_subgradient(s); /*Get the optimal variables of the Slave Problem*/
			slave.get_value(slave_cost); /*Get the optimal value of the Slave Problem*/
			slave.get_simplex_ite(simplexiter);
			slave.get_value(rhs);
			_ub += slave_cost;
			master.add_cut_slave(i_slave, s, x0, rhs);
		}

		if (_best_ub > _ub) {
			_best_ub = _ub;
			bestx = x0;
		}

		master.write(it);
		std::cout << std::setw(10) << it;
		if (_lb == -1e20)
			std::cout << std::setw(20) << "-INF";
		else
			std::cout << std::setw(20) << std::scientific << std::setprecision(10) << _lb;
		if (_ub == +1e20)
			std::cout << std::setw(20) << "+INF";
		else
			std::cout << std::setw(20) << std::scientific << std::setprecision(10) << _ub;
		if (_best_ub == +1e20)
			std::cout << std::setw(20) << "+INF";
		else
			std::cout << std::setw(20) << std::scientific << std::setprecision(10) << _best_ub;
		std::cout << std::setw(10) << simplexiter;
		std::cout << std::endl;
		if (_lb + 1e-6 >= _best_ub)
			stop = true;
	}
	std::cout << "Investment solution" << std::endl;
	for (auto const & kvp : bestx) {
		std::cout << std::setw(50) << std::left << kvp.first;
		std::cout << " = ";
		std::cout << std::setw(20) << std::scientific << std::setprecision(10) << kvp.second;
		std::cout << std::endl;
	}
}