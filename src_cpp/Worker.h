#pragma once

#include "SolverAbstract.h"
#ifdef XPRESS
#include "SolverXPRESS.h"
#endif



class Worker;
typedef std::shared_ptr<Worker> WorkerPtr;

/*!
* \class Worker
* \brief Mother-class Worker 
*
*  This class opens and sets a problem from a mps and a mapping variable map
*/

class Worker
{
public:
	bool _is_master;
	SolverAbstract::Ptr _solver;

public:
	Worker();
	void declare_solver(std::string const & solver_name);
	void init(Str2Int const & variable_map, std::string const & path_to_mps, std::string const& solver_name);
	void add_stream(std::ostream& stream);
	virtual ~Worker();

	void get_value(double & lb);
	void get_simplex_ite(int & result);

	void free();
public:
	std::string _path_to_mps;
	Str2Int _name_to_id; /*!< Link between the variable name and its identifier */
	Int2Str _id_to_name; /*!< Link between the identifier of a variable and its name*/
	
public:

	std::list<std::ostream *> & get_stream();

	void solve(int & lp_status);
	void solve_integer(int& lp_status);
	void write_errored_prob(int status, BendersOptions const& options, std::string const& path_to_mps) const;

public :
	void get_MIP_sol(double* x, double* duals);
	double get_mip_value();
};
