// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "Worker.h"
#include "Timer.h"
#include "Benders.h"
#include "SlaveCut.h"

typedef std::set<SlaveCutData> TestStorage;

int main(int argc, char** argv)
{
	Point x1;
	Point x2;
	Point x0;
	x0["a"] = 1.0;
	x0["b"] = 1.0;
	x0["c"] = 1.0;
	x1["a"] = 1.0;
	x1["b"] = 2.0;
	x1["c"] = 0.0;
	x2["a"] = 1.0;
	x2["b"] = 2.0;
	x2["c"] = 3.0;
	SlaveCutData slave_cut_data1;
	SlaveCutData slave_cut_data2;
	SlaveCutData slave_cut_data3;

	SlaveCutDataHandler handler1(slave_cut_data1);
	handler1.init();
	handler1.get_dbl(SLAVE_COST) = 10.0;
	handler1.get_point() = x1;
	SlaveCutTrimmer trimmer1(handler1, x0);
	SlaveCutDataHandler handler2(slave_cut_data2);
	handler2.init();
	handler2.get_dbl(SLAVE_COST) = 13.0;
	handler2.get_point() = x2;
	SlaveCutTrimmer trimmer2(handler2, x0);
	SlaveCutTrimmer trimmer3(handler2, x1);
	SlaveCutTrimmer trimmer4(handler2, x2);
	SlaveCutTrimmer trimmer5(handler1, x1);
	SlaveCutTrimmer trimmer6(handler1, x2);

	std::cout << "Cut1 is : " << trimmer1 << std::endl;
	std::cout << "Cut2 is : " << trimmer2 << std::endl;
	std::cout << "Cut3 is : " << trimmer3 << std::endl;
	std::cout << "Cut4 is : " << trimmer4 << std::endl;
	std::cout << "Cut5 is : " << trimmer5 << std::endl;
	std::cout << "Cut6 is : " << trimmer6 << std::endl;
	SlaveCutStorage Set;
	std::cout << std::setw(30) << trimmer1 << " inserted : " << Set.insert(trimmer1).second << std::endl;
	std::cout << std::setw(30) << trimmer2 << " inserted : " << Set.insert(trimmer2).second << std::endl;
	std::cout << std::setw(30) << trimmer3 << " inserted : " << Set.insert(trimmer3).second << std::endl;
	std::cout << std::setw(30) << trimmer4 << " inserted : " << Set.insert(trimmer4).second << std::endl;
	std::cout << std::setw(30) << trimmer5 << " inserted : " << Set.insert(trimmer5).second << std::endl;
	std::cout << std::setw(30) << trimmer6 << " inserted : " << Set.insert(trimmer6).second << std::endl;

	auto itset = Set.begin();
	std::cout << *itset << std::endl;
	handler1.init();
	handler1.get_dbl(SLAVE_COST) = 5.0;
	handler1.get_point() = x2;
	std::cout << *itset << std::endl;

	return(0);
}