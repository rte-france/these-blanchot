#include "catch2.hpp"
#include "WorkerTrace.h"

TEST_CASE("WorkerTrace", "[wrk-trace]") {

	WorkerMasterDataPtr trace(new WorkerMasterData);

	Point x;
	x["A"] = 1.0;
	x["B"] = 1.5;
	x["C"] = 2.0;
	PointPtr ptrx(new Point(x));

	trace->_x0 = ptrx;
	REQUIRE(trace->get_point() == x);

	IntVector minVec = { 0, 1, 3, 5 };
	
}