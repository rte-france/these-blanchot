#include "Benders.h"

void Benders::run() {
	_lb = -1e20;
	_ub = +1e20;
	_best_ub = +1e20;

	bool stop = false;
	size_t it(0);
	_master.write(it);
	double alpha(0);
	double slave_cost(0);
	double invest_cost(0);
	std::cout << std::setw(10) << "ITE";
	std::cout << std::setw(20) << "LB";
	std::cout << std::setw(20) << "UB";
	std::cout << std::setw(20) << "BESTUB";
	std::cout << std::endl;
	Point bestx;
	while (!stop) {
		++it;
		_master.solve();

		Point x0;
		Point s;
		double rhs;
		_master.get(x0, alpha);
		_master.get_value(_lb);
		invest_cost = _lb - alpha;
		_slave.fix_to(x0);
		//_slave.write(it);

		_slave.solve();
		_slave.get_subgradient(s);
		_slave.get_value(slave_cost);
		_ub = invest_cost + slave_cost;
		if (_best_ub > _ub) {
			_best_ub = _ub;
			bestx = x0;
		}
		_slave.get_value(rhs);

		_master.add_cut(s, x0, rhs);
		//_master.write(it);
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
		std::cout << std::endl;
		if (_lb + 1e-6 >= _best_ub)
			stop = true;
	}
	for (auto const & kvp : bestx) {
		std::cout << std::setw(20) << kvp.first;
		std::cout << " = ";
		std::cout << std::setw(20) << std::scientific << std::setprecision(10) << kvp.second;
		std::cout << std::endl;
	}
}