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

void SimplexBasisHandler::print(std::ostream & stream)const {
	std::stringstream buffer;
	buffer << "Rows : (";
	for (int i(0); i < get_row().size(); i++) {
		buffer << get_row()[i] << " ";
	}
	buffer << ") || Cols : (";
	for (int i(0); i < get_col().size(); i++) {
		buffer << get_col()[i] << " ";
	}
	buffer << ")";
	stream << buffer.str();
}

std::ostream & operator<<(std::ostream & stream, SimplexBasisHandler const & rhs) {
	rhs.print(stream);
	return stream;
}
