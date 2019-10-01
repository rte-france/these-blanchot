#pragma once

#include "Worker.h"
#include "WorkerSlave.h"

/*!
* \class WorkerSlave
* \brief Class daughter of Worker Class, build and manage a master problem
*/
class WorkerMaster;
typedef std::shared_ptr<WorkerMaster> WorkerMasterPtr;

class WorkerMaster : public Worker {
public:
	int _id_alpha;
	std::vector<int> _id_alpha_i;

	WorkerMaster();
	virtual ~WorkerMaster();

	virtual void get(Point & x0, double & alpha, DblVector & alpha_i) = 0;
	virtual void get_dual_values(std::vector<double> & dual) = 0;
	virtual int get_number_constraint() = 0;

	virtual void write(int it) = 0;

	virtual void add_cut(Point const & s, Point const & x0, double const & rhs) = 0;
	virtual void add_cut_by_iter(int const i, Point const & s, double const & sx0, double const & rhs) = 0;
	virtual void add_dynamic_cut(Point const & s, double const & sx0, double const & rhs) = 0;
	virtual void add_cut_slave(int i, Point const & s, Point const & x0, double const & rhs) = 0;
	virtual void delete_constraint(int const nrows) = 0;
	virtual void fix_alpha(double const & bestUB) = 0;

public :
	virtual void getlb_variables(BendersData & data, BendersOptions const & options, int nvars) = 0;
	virtual void getub_variables(BendersData & data, BendersOptions const & options, int nvars) = 0;

	virtual void chgbounds(int nvars, std::vector<int> index_vars, std::vector<char> bnd_types, std::vector<double> values) = 0;

};