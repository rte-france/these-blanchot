#pragma once

#include "common_mpi.h"
#include "Worker.h"
#include "WorkerSlave.h"
#include "WorkerMaster.h"


void mpi_problem_init(WorkerMaster master, mpi::environment & env, mpi::communicator & world);

void send_x0_master(WorkerMaster master, Point x0, double _lb, double _ub, mpi::environment & env, mpi::communicator & world);

void solve_slaves(mpi::environment & env, mpi::communicator & world);

void get_cut_master(mpi::environment & env, mpi::communicator & world);
