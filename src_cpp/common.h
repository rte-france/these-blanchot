#pragma once

#ifdef _MSC_VER
#pragma warning( disable : 4267 ) // implicit conversion, possible loss of data
#endif
#if defined(WIN32) || defined(_WIN32) 
#define PATH_SEPARATOR "\\" 
#else 
#define PATH_SEPARATOR "/" 
#endif 

//#define XPRESS
#define CPLEX

#include <tuple>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <memory>
#include <algorithm>
#include <thread>
#include "Timer.h"

struct Predicate;
typedef std::map<std::string, double> Point;

typedef std::shared_ptr<Point> PointPtr;

double const EPSILON_PREDICATE = 1e-8;

typedef std::set<std::string> problem_names;
typedef std::map<std::string, int> Str2Int;
typedef std::map<int, std::string> Int2Str;
typedef std::map<std::string, double> Str2Dbl;
typedef std::vector<int> IntVector;
typedef std::vector<char> CharVector;
typedef std::vector<double> DblVector;
typedef std::vector<std::string> StrVector;
typedef std::map < std::string, Str2Int> CouplingMap;

typedef std::set<std::string> StrSet;

typedef std::map <std::string, IntVector> SlaveCutId;
//typedef std::tuple <int, std::string, int, bool> ActiveCut;
//typedef std::vector<ActiveCut> ActiveCutStorage;

typedef std::pair<std::string, std::string> mps_coupling;
typedef std::list<mps_coupling> mps_coupling_list;
typedef std::pair<std::string, std::string> StrPair;
typedef std::map<StrPair, double> StrPair2Dbl;

typedef std::vector<StrPair> StrPairVector;

/*!
* \class Predicate
* \brief TO DO
*/
struct Predicate {
	bool operator()(PointPtr const & lhs, PointPtr const & rhs)const {
		return *lhs < *rhs;
	}

	bool operator()(Point const & lhs, Point const & rhs)const {
		Point::const_iterator it1(lhs.begin());
		Point::const_iterator it2(rhs.begin());

		Point::const_iterator end1(lhs.end());
		Point::const_iterator end2(rhs.end());

		while (it1 != end1 && it2 != end2) {
			if (it1->first != it2->first) {
				return it1->first < it2->first;
			}
			else {
				if (std::fabs(it1->second - it2->second) < EPSILON_PREDICATE) {
					it1++;
					it2++;
				}
				else {
					return it1->second < it2->second;
				}
			}
		}

		if (it1 == end1 && it2 == end2) {
			return false;
		}
		else {
			return (it1 == end1);
		}
	}
};


/*!
*  \brief Stream output overloading for point
*
*  \param stream : stream output
*
*  \param rhs : point
*/
inline std::ostream & operator<<(std::ostream & stream, Point const & rhs) {
	for (auto const & kvp : rhs) {
		if (kvp.second > 0) {
			if (kvp.second == 1) {
				stream << "+";
				stream << kvp.first;
			}
			else {
				stream << "+";
				stream << kvp.second;
				stream << kvp.first;
			}
		}
		else if (kvp.second < 0) {
			stream << kvp.second;
			stream << kvp.first;
		}
	}
	return stream;
}

/*!
* \struct BendersData
* \brief Structure used to manage every benders data
*/
struct BendersData {
	double timer_slaves;			/*!< Time spent in the slave resolution at each iteration */
	double timer_master;			/*!< Time spent in the master resolution at each iteration */
	double lb;						/*!< Lower bound of Benders resolution */
	double ub;						/*!< Upper bound computed at the current iteration */
	double best_ub;					/*!< Best upper bound found */
	int maxsimplexiter;				/*!< Number max of simplex iterations observed in the slave resolutions */
	int minsimplexiter;				/*!< Number min of simplex iterations observed in the slave resolutions */
	int deletedcut;					/*!< Number of cuts not added at this iteration because they 
									already exixst in Benders */
	int it;							/*!< Iteration of the algorithm */
	bool stop;						/*!< True if one of the stopping criterion is observed */
	double alpha;					/*!< Value of the weighted sum of the epigraph variables of the subproblems */
	std::vector<double> alpha_i;	/*!< Vector of the value of the epigrpah variables of the subproblems */
	double slave_cost;				/*!< Second stage cost at this iteration */
	double invest_cost;				/*!< First stage cost of the master solution at this iteration */
	double invest_separation_cost;	/*!< FIrst stage cost of the separation point*/
	Point bestx;					/*!< Best point observed (lowest upper bound) */
	Point x0;						/*!< Current solution of master problem */
	int nslaves;					/*!< Number of subproblems */

	int master_status;				/*!< Solver status after master resolution */
	int slave_status;				/*!< Worst solver status after resolution of the subproblems */
	int global_prb_status;			/*!< Status of the entire problem after resolution of master and subproblems */

	// Stabilisation in-out
	Point x_cut;					/*!< Point in which a cut is computed */
	Point x_stab;					/*!< Stability center used in in-out stabilisation */
	double stab_value;				/*!< Value of the stabilisation (between 0 and 1) */

	Timer total_time;				/*!< Total time elapsed */

	// Enhanced multicut
	int nrandom;					/*!< Number of slaves problems to sample randomly at one iteration if needed (OUTDATED) */
	int batch_size;					/*!< Number max of slaves to solve on each process (= total number of slaves
									in sequential mode ) */
	int n_slaves_no_cut;			/*< Counter of slaves solved in a particular first stage solution
									which were not cut */
	double espilon_s;				/*!< optimality gap on one subproblem */
	bool has_cut;					/*!< Bool saying if a subproblem has been cut at the last iteration */
	IntVector indices;				/*!< Vector of indices of subproblems to perform sampling, the order of this 
									vector will tell the subproblems to sample */
	double step_size;				/*!< Step size taken by enhanced multicut
									x(k) = x(k-1) + step_size* ( xMaster - x(k-1) )*/

	DblVector last_value;			/*!< Last value observed of a subproblem */

	int nocutmaster;				/*!< Number of time the master is not cut */
	int misprices;					/*!< Number if successive misprices */
	double epsilon_x;
	bool stay_in_x_cut;
	int n_slaves_solved;
	Point x_mem;


};

double norm_point(Point const & x0, Point const & x1);
int norm_int(IntVector const & x0, IntVector const & x1);

std::ostream & operator<<(std::ostream & stream, std::vector<IntVector> const & rhs);