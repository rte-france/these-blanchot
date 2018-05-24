#include "WorkerStabilizedMaster.h"



WorkerStabilizedMaster::WorkerStabilizedMaster():WorkerMaster() {

}
WorkerStabilizedMaster::WorkerStabilizedMaster(std::string const & problem_name, DblVector const & slave_weight, int nslaves ) : WorkerMaster(problem_name, slave_weight, nslaves) {

}
WorkerStabilizedMaster::~WorkerStabilizedMaster() {

}
