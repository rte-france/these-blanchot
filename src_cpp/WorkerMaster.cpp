#include "WorkerMaster.h"

WorkerMaster::WorkerMaster() {
}


WorkerMaster::~WorkerMaster() {
}

/*!
*  \brief Return optimal variables of a problem
*
*  Set optimal variables of a problem which has the form (min(x,alpha) : f(x) + alpha)
*
*  \param x0 : reference to an empty map list
*
*  \param alpha : reference to an empty double
*/
void WorkerMaster::get(Point & x0, double & alpha, DblVector & alpha_i) {
	x0.clear();
	std::vector<double> ptr(get_ncols(), 0.0);
	_solver->get_MIP_sol(ptr.data(), NULL);

	for (auto const & kvp : _id_to_name) {
		x0[kvp.second] = ptr[kvp.first];
	}
	alpha = ptr[_id_alpha];
	for (int i(0); i < _id_alpha_i.size(); ++i) {
		alpha_i[i] = ptr[_id_alpha_i[i]];
	}
}

/*!
*  \brief Set dual values of a problem in a vector
*
*  \param dual : reference to a vector of double -- getmipsol : slack and no duals
*/
void WorkerMaster::get_dual_values(std::vector<double> & dual) {
	int rows = _solver->get_nrows();
	dual.resize(rows);
	_solver->get_MIP_sol(NULL, dual.data());
}

/*!
*  \brief Return number of constraint in a problem
*/
int WorkerMaster::get_number_constraint() {
	int rows = _solver->get_nrows();
	return rows;
}

/*!
*  \brief Delete nrows last rows of a problem
*
*  \param nrows : number of rows to delete
*/
void WorkerMaster::delete_constraint(int const nrows) {
	int const nconstraint(get_number_constraint());
	_solver->del_rows(nconstraint - nrows, nconstraint - 1);
}


/*!
*  \brief Write a problem in a lp file
*
*  \param it : number of the problem
*/

void WorkerMaster::write(int it) {
	std::stringstream name;
	name << "master_" << it << ".lp";
	_solver->write_prob(name.str().c_str(), "l");
}

/*!
*  \brief Add benders cut to a problem
*
*  \param s : optimal slave variables
*  \param x0 : optimal Master variables
*  \param rhs : optimal slave value
*/
void WorkerMaster::add_cut(Point const & s, Point const & x0, double const & rhs) {
	// cut is -rhs >= alpha  + s^(x-x0)
	int nrows(1);
	int ncoeffs(1 + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 1);
	std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;
	for (auto const & kvp : _name_to_id) {
		rowrhs.front() += (s.find(kvp.first)->second * x0.find(kvp.first)->second);
		mclind[kvp.second] = kvp.second;
		matval[kvp.second] = (1.0/_id_alpha_i.size() ) * s.find(kvp.first)->second;
	}
	rowrhs.front() *= (1.0 / _id_alpha_i.size());

	mclind.back() = _id_alpha;
	matval.back() = -1;
	mstart.back() = (int)matval.size();

	_solver->add_rows(nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
}

/*!
*  \brief Add benders cut to a problem
*
*  \param s : optimal slave variables
*  \param sx0 : subgradient times x0
*  \param rhs : optimal slave value
*/
void WorkerMaster::add_dynamic_cut(Point const & s, double const & sx0, double const & rhs) {
	// cut is -rhs >= alpha  + s^(x-x0)
	int nrows(1);
	int ncoeffs(1 + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 1);
	std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;
	rowrhs.front() += sx0;

	for (auto const & kvp : _name_to_id) {
		mclind[kvp.second] = kvp.second;
		matval[kvp.second] = s.find(kvp.first)->second;
	}

	mclind.back() = _id_alpha;
	matval.back() = -1;
	mstart.back() = (int)matval.size();

	_solver->add_rows(nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
}

/*!
*  \brief Add benders cut to a problem
*
*  \param i : identifier of a slave problem
*  \param s : optimal slave variables
*  \param sx0 : subgradient times x0
*  \param rhs : optimal slave value
*/
void WorkerMaster::add_cut_by_iter(int const i, Point const & s, double const & sx0, double const & rhs) {
	// cut is -rhs >= alpha  + s^(x-x0)
	int nrows(1);
	int ncoeffs(1 + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 1);
	std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;
	rowrhs.front() += sx0;

	for (auto const & kvp : _name_to_id) {
		mclind[kvp.second] = kvp.second;
		matval[kvp.second] = s.find(kvp.first)->second;
	}
	mclind.back() = _id_alpha_i[i];
	matval.back() = -1;
	mstart.back() = (int)matval.size();

	_solver->add_rows(nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
}


/*!
*  \brief Add one benders cut to a problem
*
*  \param i : identifier of a slave problem
*  \param s : optimal slave variables
*  \param x0 : optimal Master variables
*  \param rhs : optimal slave value
*/
void WorkerMaster::add_cut_slave(int i, Point const & s, Point const & x0, double const & rhs) {
	// cut is -rhs >= alpha  + s^(x-x0)
	int nrows(1);
	int ncoeffs(1 + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 0);
	std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;

	for (auto const & kvp : _name_to_id) {
		if (s.find(kvp.first) != s.end()) {
			//std::cout << kvp.first << "   " << x0.find(kvp.first)->second << "   " << s.find(kvp.first)->second << std::endl;
			rowrhs.front() += s.find(kvp.first)->second * x0.find(kvp.first)->second;
			mclind[kvp.second] = kvp.second;
			matval[kvp.second] = s.find(kvp.first)->second;
		}
		else {
			//std::cout << kvp.first << "   " << x0.find(kvp.first)->second << "   " << 0.0 << std::endl;
			rowrhs.front() += 0.0;
			mclind[kvp.second] = kvp.second;
			matval[kvp.second] = 0.0;
		}
	}

	mclind.back() = _id_alpha_i[i];
	matval.back() = -1;
	mstart.back() = (int)matval.size();

	_solver->add_rows(nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
}

void WorkerMaster::add_agregated_cut_slaves(IntVector const& ids, Point const& s, Point const& x0, double const& rhs)
{
	// cut is -rhs >= alpha  + s^(x-x0)
	int nrows(1);
	int ncoeffs((int)ids.size() + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 0);
	std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;

	for (auto const& kvp : _name_to_id) {
		if (s.find(kvp.first) != s.end()) {
			rowrhs.front() += s.find(kvp.first)->second * x0.find(kvp.first)->second;
			mclind[kvp.second] = kvp.second;
			matval[kvp.second] = s.find(kvp.first)->second;
		}
		else {
			//std::cout << kvp.first << "   " << x0.find(kvp.first)->second << "   " << 0.0 << std::endl;
			rowrhs.front() += 0.0;
			mclind[kvp.second] = kvp.second;
			matval[kvp.second] = 0.0;
		}
	}

	for (int i(0); i < (int)ids.size(); i++) {
		mclind[i + _name_to_id.size()] = _id_alpha_i[ids[i]];
		matval[i + _name_to_id.size()] = -1;
	}
	mstart.back() = (int)matval.size();

	/*for (auto const& val : matval) {
		std::cout << val << "  ";
	}
	std::cout << std::endl << std::endl;

	for (auto const& val : mstart) {
		std::cout << val << "  ";
	}
	std::cout << std::endl << std::endl;

	for (auto const& val : mclind) {
		std::cout << val << "  ";
	}
	std::cout << std::endl << std::endl;*/

	_solver->add_rows(nrows, ncoeffs, rowtype.data(), rowrhs.data(), NULL, mstart.data(), mclind.data(), matval.data());
}


/*!
*  \brief Constructor of a Master Problem
*
*  Construct a Master Problem by loading mps and mapping files and adding the variable alpha
*
*  \param variable_map : map linking each variable to its id in the problem
*  \param path_to_mps : path to the problem mps file
*  \param options : set of benders options
*  \param nslaves : number of slaves
*/
WorkerMaster::WorkerMaster(Str2Int const & variable_map, std::string const & path_to_mps, BendersOptions const & options, int nslaves) :Worker() {
	init(variable_map, path_to_mps, options.SOLVER);
	_is_master = true;
	_id_level_constraint = -1;

	_solver->set_output_log_level(options.XPRESS_TRACE);

	// 4 barrier
	// 2 dual
	_solver->set_algorithm(options.MASTER_METHOD);

	// add the variable alpha
	std::string const alpha("alpha");
	_id_alpha = 0;
	auto const it(_name_to_id.find(alpha));
	if (it == _name_to_id.end()) {
		double lb(options.THETA_LB); /*!< Lower Bound */
		double global_lb = lb * nslaves;
		double ub(+1e20); /*!< Upper Bound*/
		double obj(+1);
		double zero(0);
		std::vector<int> start(2, 0);
		_id_alpha = _solver->get_ncols(); /* Set the number of columns in _id_alpha */
		_solver->add_cols(1, 0, &obj, start.data(), NULL, NULL, &global_lb, &ub); /* Add variable alpha and its parameters */
		_solver->add_name(2, alpha.c_str(), _id_alpha);

		_id_alpha_i.resize(nslaves, -1);

		for (int i(0); i < nslaves; ++i) {
			_id_alpha_i[i] = _solver->get_ncols();
			_solver->add_cols(1, 0, &zero, start.data(), NULL, NULL, &lb, &ub); /* Add variable alpha_i and its parameters */
			std::stringstream buffer;
			buffer << "alpha_" << i;
			_solver->add_name(2, buffer.str().c_str(), _id_alpha_i[i]);
		}

		// Constraint theta = sum{theta_i}
		std::vector<char> rowtype(1, 'E');
		std::vector<double> rowrhs(1, 0);
		std::vector<double> matval(nslaves+1, 0);
		std::vector<int> mclind(nslaves + 1);
		std::vector<int> mstart(1 + 1, 0);
		mclind[0] = _id_alpha;
		matval[0] = 1;

		for (int i(0); i < nslaves; ++i) {
			mclind[i + 1] = _id_alpha_i[i];
			matval[i + 1] = -(1.0/nslaves);
			//std::cout << matval[i + 1] << std::endl;
		}
		_solver->add_rows(1, nslaves + 1, rowtype.data(), rowrhs.data(), NULL, 
			mstart.data(), mclind.data(), matval.data());
			
		// bundle level constraint "(1/nslaves).theta + c^T.x <= lev" if applicable, 
		// initialisazion of level with a 0 RHS
		save_obj();
		if (options.ALGORITHM == "LEVEL") {
			int nvars = _solver->get_ncols() - nslaves - 1;

			// Modif problem type to QP
			_solver->set_problem_to_quadratic();

			// saving and removing old objective function
			remove_obj();

			// Passing matrix identity as QP matrix to problem
			set_matrix_identity(nvars);

			// Add level constraint
			rowtype[0] = 'L';
			rowrhs[0] = 0;

			mstart[0] = 0;
			mstart[1] = nvars + 1;
			
			matval.resize(nvars + 1);
			mclind.resize(nvars + 1);
			for (int i = 0; i < nvars; i++) {
				matval[i] = _initial_obj[i];
				mclind[i] = i;
			}
			matval[nvars] = 1.0;//(1.0 / nslaves);
			mclind[nvars] = _id_alpha;

			_solver->add_rows(1, nvars + 1, rowtype.data(), rowrhs.data(), NULL,
				mstart.data(), mclind.data(), matval.data());

			_id_level_constraint = _solver->get_nrows() - 1;
		}
	}
	else {
		std::cout << "ERROR a variable named alpha is in input" << std::endl;
	}
}

/*!
*  \brief Fix an upper bound and the variable alpha of a problem
*
*  \param bestUB : bound to fix
*/
void WorkerMaster::fix_alpha(double const & bestUB) {
	std::vector<char> boundtype(1, 'U');
	_solver->chg_bounds(1, &_id_alpha, boundtype.data(), &bestUB);
}

void WorkerMaster::remove_obj()
{
	int nvars = _solver->get_ncols();
	IntVector index(nvars);
	DblVector zeros(nvars, 0.0);

	for (int i = 0; i < nvars; i++) {
		index[i] = i;
	}

	_solver->chg_obj(nvars, index.data(), zeros.data());
}

void WorkerMaster::set_problem_to_quadratic()
{
	_solver->set_problem_to_quadratic();
}

void WorkerMaster::update_level_objective(Point const& center)
{
	int n_master_vars = center.size();

	DblVector elts(n_master_vars);
	IntVector indices(n_master_vars);

	int i = 0;
	for (auto const& kvp : center) {
		indices[i] = _name_to_id[kvp.first];
		elts[i] = -kvp.second;
		i++;
	}
	_solver->chg_obj(n_master_vars, indices.data(), elts.data());
}

void WorkerMaster::update_level_constraint(double const& lev)
{
	_solver->chg_rhs(_id_level_constraint, lev);
}

void WorkerMaster::set_matrix_identity(int n_first_vars)
{
	for (int i(0); i < n_first_vars; i++) {
		_solver->chg_quadratic_coef(i, i, 1);
	}
}

void WorkerMaster::save_obj()
{
	int n_master_vars = _solver->get_ncols() - _id_alpha_i.size() - 1;
	_initial_obj.resize(n_master_vars);
	_solver->get_obj(_initial_obj.data(), 0, n_master_vars - 1);
}
