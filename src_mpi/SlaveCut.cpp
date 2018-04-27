#include "common_mpi.h"
#include "SlaveCut.h"


//SlaveCut::SlaveCut(){
//
//
//}
//
//SlaveCut::~SlaveCut() {
//
//}

//void build_SlaveCutData(SlaveCutData & rhs) {
//
//}
SlaveCutDataHandler::SlaveCutDataHandler(SlaveCutData & data):_data(data){

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