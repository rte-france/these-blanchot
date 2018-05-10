#include "common_mpi.h"
#include "SlaveCut.h"



SlaveCutDataHandler::SlaveCutDataHandler(SlaveCutData & data) :_data(data) {

}
SlaveCutDataHandler::~SlaveCutDataHandler() {

}
void SlaveCutDataHandler::init() {
	get_point().clear();
	get_int().resize(SlaveCutInt::MAXINT);
	get_dbl().resize(SlaveCutDbl::MAXDBL);
	get_str().resize(SlaveCutStr::MAXSTR);
}
Point & SlaveCutDataHandler::get_point() {
	return _data.first.first.first;
}
IntVector & SlaveCutDataHandler::get_int() {
	return _data.first.first.second;
}
DblVector & SlaveCutDataHandler::get_dbl() {
	return _data.first.second;

}
StrVector & SlaveCutDataHandler::get_str() {
	return _data.second;

}
int & SlaveCutDataHandler::get_int(SlaveCutInt key) {
	return get_int()[key];
}
double & SlaveCutDataHandler::get_dbl(SlaveCutDbl  key) {
	return get_dbl()[key];
}
std::string & SlaveCutDataHandler::get_str(SlaveCutStr key) {
	return get_str()[key];
}


Point const & SlaveCutDataHandler::get_point() const {
	return _data.first.first.first;
}
IntVector const & SlaveCutDataHandler::get_int() const {
	return _data.first.first.second;
}
DblVector const & SlaveCutDataHandler::get_dbl() const {
	return _data.first.second;

}
StrVector const & SlaveCutDataHandler::get_str() const {
	return _data.second;

}
int SlaveCutDataHandler::get_int(SlaveCutInt key)const {
	return get_int()[key];
}
double SlaveCutDataHandler::get_dbl(SlaveCutDbl  key) const {
	return get_dbl()[key];
}
std::string const & SlaveCutDataHandler::get_str(SlaveCutStr key) const {
	return get_str()[key];
}


bool SlaveCutTrimmer::operator<(SlaveCutTrimmer const & other) const {
	Predicate point_comp;
	//return((get_const_cut() < other.get_const_cut()) || ((std::fabs(get_const_cut() == other.get_const_cut()) < EPSILON_PREDICATE)) && (point_comp(_x0, other._x0)));
	if (std::fabs(get_const_cut() - other.get_const_cut()) < EPSILON_PREDICATE) {
		return point_comp(_data_cut.get_point(), other._data_cut.get_point());
	}
	else {
		return (get_const_cut() < other.get_const_cut());
	}
}


SlaveCutTrimmer::SlaveCutTrimmer(SlaveCutDataHandler const & data, Point const & x0) : _data_cut(data), _x0(x0) {
}


double SlaveCutTrimmer::get_const_cut()const {
	double result(_data_cut.get_dbl(SLAVE_COST));
	for (auto const & kvp : _x0) {
		result -= _data_cut.get_point().find(kvp.first)->second * _x0.find(kvp.first)->second;
	}
	return result;
}

std::ostream & operator<<(std::ostream & stream, SlaveCutTrimmer const & rhs) {
	rhs.print(stream);
	return stream;
}
void SlaveCutTrimmer::print(std::ostream & stream)const {
	stream << " |  Constant " << get_const_cut() << " Coeff " << _data_cut.get_point() << " | ";
}

std::ostream & operator<<(std::ostream & stream, SlaveCutData const & rhs) {
	stream << " | Subgradient " << rhs.first.first.first;
	if (&rhs.first.first.second[SIMPLEXITER] != NULL) {
		stream << " Simplexiter " << rhs.first.first.second[SIMPLEXITER];
	}
	if (&rhs.first.first.second[SIMPLEXITER] != NULL) {
		stream << " Slave cost " << rhs.first.second[SLAVE_COST];
	}
	stream << " | ";
	return stream;
}