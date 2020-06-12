#pragma once

#include "Worker.h"
#include "SlaveCut.h"


/*! 
* \class WorkerSlave
* \brief Class daughter of Worker Class, build and manage a slave problem
*/
class WorkerSlave;
typedef std::shared_ptr<WorkerSlave> WorkerSlavePtr;
typedef std::vector<WorkerSlavePtr> WorkerSlaves;
typedef std::map<std::string, WorkerSlavePtr> SlavesMapPtr;


class WorkerSlave : public Worker {
public :
	double _proba;

public:

	WorkerSlave();
	WorkerSlave(Str2Int const & variable_map, std::string const & path_to_mps, 
		double const & slave_weight, BendersOptions const & options);
	WorkerSlave(Str2Int const& variable_map, std::string const& path_to_mps,
		double const& slave_weight, BendersOptions const& options, StrPairVector keys, 
		DblVector values, WorkerPtr fictif);
	virtual ~WorkerSlave();

	void set_realisation_to_prob(StrPairVector keys, DblVector values);

public:
	void write(int it);
	void fix_to(Point const & x0);
	void get_subgradient(Point & s);

public :
	// Surcharge avec multiplication par _proba
	void get_value(double& lb);
};


