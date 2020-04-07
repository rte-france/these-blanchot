#pragma once

#include "WorkerMaster.h"
#include "BendersOptions.h"
#include "BendersFunctions.h"


/*!
* \class Benders
* \brief Class use run the benders algorithm in sequential
*/
class Benders {
public:
	Benders(CouplingMap const & problem_list, BendersOptions const & options);
	virtual ~Benders();

	void free();

	void build_cut();
	void run(std::ostream& stream);

	void classic_iteration(std::ostream& stream);
	void enhanced_multicut_iteration(std::ostream& stream);

public:
	WorkerMasterPtr _master; /*!<  Pointer to master problem */
	SlavesMapPtr _map_slaves; /*!< Map of slaves problems <string, WorkerSlavePre> */

	Str2Int _problem_to_id; /*!< Map linking problems to the id of their epigraph variables */
	BendersData _data; /*!< Data of the resolution */
	BendersOptions _options; /*!<  Options */
	StrVector _slaves; /*!< Vector of subproblems names */

	AllCutStorage _all_cuts_storage; /*!< Storage of cuts */

	SlaveCutId _slave_cut_id; /*!< id of the cuts */
};