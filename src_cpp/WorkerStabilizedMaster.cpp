#include "WorkerStabilizedMaster.h"



WorkerStabilizedMaster::WorkerStabilizedMaster():WorkerMaster() {

}
WorkerStabilizedMaster::WorkerStabilizedMaster(std::map<std::string, int> const & variable_name, std::string const & problem_name, DblVector const & slave_weight, int nslaves) : WorkerMaster(variable_name, problem_name, slave_weight, nslaves) {

}
WorkerStabilizedMaster::~WorkerStabilizedMaster() {

}
