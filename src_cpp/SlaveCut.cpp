#include "SlaveCut.h"


SlaveCutDataHandler::SlaveCutDataHandler(SlaveCutDataPtr & data) :_data(data) {
	get_subgradient().clear();
	get_int().resize(SlaveCutInt::MAXINT);
	get_dbl().resize(SlaveCutDbl::MAXDBL);
	get_str().resize(SlaveCutDbl::MAXDBL);
}

SlaveCutDataHandler::~SlaveCutDataHandler() {

}

Point & SlaveCutDataHandler::get_subgradient() {
	return _data->first.first.first;
}
IntVector & SlaveCutDataHandler::get_int() {
	return _data->first.first.second;
}
DblVector & SlaveCutDataHandler::get_dbl() {
	return _data->first.second;
}
StrVector & SlaveCutDataHandler::get_str() {
	return _data->second;
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


Point const & SlaveCutDataHandler::get_subgradient() const {
	return _data->first.first.first;
}
IntVector const & SlaveCutDataHandler::get_int() const {
	return _data->first.first.second;
}
DblVector const & SlaveCutDataHandler::get_dbl() const {
	return _data->first.second;

}
StrVector const & SlaveCutDataHandler::get_str() const {
	return _data->second;
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
		return point_comp(_data_cut->get_subgradient(), other._data_cut->get_subgradient());
	}
	else {
		return (get_const_cut() < other.get_const_cut());
	}
}


SlaveCutTrimmer::SlaveCutTrimmer(SlaveCutDataHandlerPtr & data, Point & x0) : _data_cut(data), _x0(x0) {
}


double SlaveCutTrimmer::get_const_cut()const {
	double result(_data_cut->get_dbl(SLAVE_COST));
	for (auto const & kvp : _x0) {
		result -= get_subgradient().find(kvp.first)->second * kvp.second;
	}
	return result;
}

Point const & SlaveCutTrimmer::get_subgradient() const {
	return _data_cut->get_subgradient();
}

std::ostream & operator<<(std::ostream & stream, SlaveCutTrimmer const & rhs) {
	rhs.print(stream);
	return stream;
}
void SlaveCutTrimmer::print(std::ostream & stream)const {
	std::stringstream buffer;
	buffer << get_const_cut() << get_subgradient();
	stream << buffer.str();
}

std::ostream & operator<<(std::ostream & stream, SlaveCutDataHandler const & rhs) {
	rhs.print(stream);
	return stream;
}
void SlaveCutDataHandler::print(std::ostream & stream)const {
	std::stringstream buffer;
	buffer << get_dbl(SLAVE_COST) << get_subgradient();
	stream << buffer.str();
	if (get_int(SIMPLEXITER) != NULL) {
		stream << " Simplexiter " << get_int(SIMPLEXITER) << " | ";
	}
}

std::ostream & operator<<(std::ostream & stream, SlaveCutData const & rhs) {
	std::stringstream buffer;
	buffer << rhs.first.second[SLAVE_COST] << rhs.first.first.first;
	stream << buffer.str();
	if (&rhs.first.first.second[SIMPLEXITER] != NULL) {
		stream << " Simplexiter " << rhs.first.first.second[SIMPLEXITER] << " | ";
	}
	return stream;
}