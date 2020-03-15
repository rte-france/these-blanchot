#pragma once

#include "launcher.h"
#include "Worker.h"
#include "BendersOptions.h"
#include "BendersFunctions.h"

class WorkerMerge;
typedef std::shared_ptr<WorkerMerge> WorkerMergePtr;

/*!
* \class WorkerMerge
* \brief Class daughter of Worker used in merge_mps to set up and solve the deterministic reformulation
*/
class WorkerMerge : public Worker {
public:
	Str2Int _decalage;		/*!< number of existing columns in the full problem to add the new ones at the end */
	int _ncols;				/*!< Number total of column in the full problem */
	int _nslaves;			/*!< Number of subproblems */
	CouplingMap _x_mps_id;	/*!< id of first stage variable in each mps file */

public:
	WorkerMerge(BendersOptions const& options);
	WorkerMerge(BendersOptions const & options, CouplingMap const& input, std::string const& name);
	~WorkerMerge();
	void free();

public:
	void read(std::string const& path_to_mps, std::string const& flags);
	void write_prob(std::string const& name, std::string const& flags);
	void fill_mps_id(std::pair<std::string, Str2Int> first_stage_vars);
	void merge_problems(CouplingMap const& input, BendersOptions const& options);

public:
	void get_obj(DblVector& obj, int first, int last);
	void get_optimal_point_and_value(Point& x0, double& val, CouplingMap & input, BendersOptions const& options);
	void chg_obj(BendersOptions const& options, double weight);
	void set_decalage(std::string const& prb);
	void add_coupling_constraints();

public:
	int get_ncols();
	void set_threads(int n_threads);
};


/*!
* \class StandardLp
* \brief Class used to set a subproblem and add it in the full problem
*/
class StandardLp {
private:
	// to be used in boost serialization for mpi transfer
	raw_standard_lp_data _data;
public:
	void init() {
		std::get<Attribute::INT_INDEX>(_data).assign(IntAttribute::MAX_INT_ATTRIBUTE, 0);

		std::get<Attribute::INT_VECTOR_INDEX>(_data).assign(IntVectorAttribute::MAX_INT_VECTOR_ATTRIBUTE, IntVector());
		std::get<Attribute::CHAR_VECTOR_INDEX>(_data).assign(CharVectorAttribute::MAX_CHAR_VECTOR_ATTRIBUTE, CharVector());
		std::get<Attribute::DBL_VECTOR_INDEX>(_data).assign(DblVectorAttribute::MAX_DBL_VECTOR_ATTRIBUTE, DblVector());
	}
	StandardLp(WorkerMerge& prob) {
		init();

		std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS] = prob._solver->get_ncols();
		std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NROWS] = prob._solver->get_nrows();
		std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NELES] = prob._solver->get_nelems();

		std::get<Attribute::INT_VECTOR_INDEX>(_data)[IntVectorAttribute::MSTART].assign(
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NROWS] + 1, 0);
		std::get<Attribute::INT_VECTOR_INDEX>(_data)[IntVectorAttribute::MINDEX].assign(
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NELES], 0);

		std::get<Attribute::CHAR_VECTOR_INDEX>(_data)[CharVectorAttribute::COLTYPE].assign(
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS], 0);
		std::get<Attribute::CHAR_VECTOR_INDEX>(_data)[CharVectorAttribute::ROWTYPE].assign(
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NROWS], 'E');

		std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::MVALUE].assign(
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NELES], 0);
		std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::RHS].assign(
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NROWS], 0);
		std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::RANGE].assign(
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NROWS], 0);

		std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::OBJ].assign(
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS], 0);
		std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::LB].assign(
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS], 0);
		std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::UB].assign(
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS], 0);

		int ncoeffs(0);

		prob._solver->get_rows(std::get<Attribute::INT_VECTOR_INDEX>(_data)[IntVectorAttribute::MSTART].data(),
			std::get<Attribute::INT_VECTOR_INDEX>(_data)[IntVectorAttribute::MINDEX].data(),
			std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::MVALUE].data(),
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NELES], &ncoeffs,
			0, std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NROWS] - 1);

		prob._solver->get_row_type(std::get<Attribute::CHAR_VECTOR_INDEX>(_data)[CharVectorAttribute::ROWTYPE].data(),
			0, std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NROWS] - 1);
		prob._solver->get_rhs(std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::RHS].data(),
			0, std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NROWS] - 1);
		prob._solver->get_rhs_range(std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::RANGE].data(),
			0, std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NROWS] - 1);

		prob._solver->get_col_type(std::get<Attribute::CHAR_VECTOR_INDEX>(_data)[CharVectorAttribute::COLTYPE].data(),
			0, std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS] - 1);

		prob._solver->get_lb(std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::LB].data(),
			0, std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS] - 1);
		prob._solver->get_ub(std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::UB].data(),
			0, std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS] - 1);
		prob._solver->get_obj(std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::OBJ].data(),
			0, std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS] - 1);
	}

	int append_in(WorkerMerge& prob) const {
		IntVector newmindex(std::get<Attribute::INT_VECTOR_INDEX>(_data)[IntVectorAttribute::MINDEX]);

		int ncols = prob.get_ncols();
		int newcols = std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS];

		IntVector newcindex(newcols);

		// symply increment the columns indexes
		for (auto& i : newmindex) {
			i += ncols;
		}

		for (int i = 0; i < newcols; i++) {
			newcindex[i] = i + ncols;
		}

		prob._solver->add_cols(std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS],
			0, std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::OBJ].data(),
			NULL, NULL, NULL, std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::LB].data(),
			std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::UB].data());

		prob._solver->chg_col_type(std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NCOLS],
			newcindex.data(), std::get<Attribute::CHAR_VECTOR_INDEX>(_data)[CharVectorAttribute::COLTYPE].data());

		std::cout << "COL TYPES = " << std::get<Attribute::CHAR_VECTOR_INDEX>(_data)[CharVectorAttribute::COLTYPE].data() << std::endl;

		prob._solver->add_rows(std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NROWS],
			std::get<Attribute::INT_INDEX>(_data)[IntAttribute::NELES],
			std::get<Attribute::CHAR_VECTOR_INDEX>(_data)[CharVectorAttribute::ROWTYPE].data(),
			std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::RHS].data(),
			std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::RANGE].data(),
			std::get<Attribute::INT_VECTOR_INDEX>(_data)[IntVectorAttribute::MSTART].data(),
			newmindex.data(), std::get<Attribute::DBL_VECTOR_INDEX>(_data)[DblVectorAttribute::MVALUE].data());

		return ncols;
	}
};