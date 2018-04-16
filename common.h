#pragma once


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

struct Predicate;
typedef std::map<std::string, double> Point;

typedef std::shared_ptr<Point> PointPtr;

typedef std::pair<double, PointPtr> Cut;

typedef std::set<Cut, Predicate> Cuts;

typedef std::shared_ptr<Cuts> CutsPtr;

double const EPSILON_PREDICATE = 1e-8;

struct Predicate {
	bool operator()(Cut const & lhs, Cut const & rhs)const {
		if (std::fabs(lhs.first - rhs.first) > EPSILON_PREDICATE) {
			return lhs.first > rhs.first;
		}
		else {
			return lhs.second < rhs.second;
		}
	}
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
			return it1 == end1;
		}
	}
};
