#include "SimplexBasis.h"


SimplexBasisHandler::SimplexBasisHandler(SimplexBasisPtr & data) :_data(data) {
}

SimplexBasisHandler::~SimplexBasisHandler() {

}

bool SimplexBasisHandler::operator<(SimplexBasisHandler const & other) const
{
	return((get_row() < other.get_row()) || ((get_row() == other.get_row()) && (get_col() < other.get_col())));
}


IntVector & SimplexBasisHandler::get_col() {
	return _data->second;
}

IntVector & SimplexBasisHandler::get_row() {
	return _data->first;
}

IntVector const & SimplexBasisHandler::get_col() const {
	return _data->second;
}

IntVector const & SimplexBasisHandler::get_row() const {
	return _data->first;
}