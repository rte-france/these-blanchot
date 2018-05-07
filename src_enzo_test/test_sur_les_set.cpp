// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "Worker.h"
#include "Timer.h"
#include "Benders.h"
#include "SlaveCut.h"
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
	std::cout << " t1 < t2 : " << (trimmer1 < trimmer2) << " and t2 < t1 : " << (trimmer2 < trimmer1) << std::endl;


	SlaveCutStorage Set;
	std::pair<std::set<SlaveCutTrimmer>::iterator, bool> result;
	result = Set.insert(trimmer1);
	std::cout << "Cut1 : " << trimmer1 << " inserted : " << result.second << std::endl;
	result = Set.insert(trimmer2);
	std::cout << "Cut2 : " << trimmer2 << " inserted : " << result.second << std::endl;
	result = Set.insert(trimmer3);
	std::cout << "Cut3 : " << trimmer3 << " inserted : " << result.second << std::endl;
	result = Set.insert(trimmer4);
	std::cout << "Cut4 : " << trimmer4 << " inserted : " << result.second << std::endl;
	result = Set.insert(trimmer5);
	std::cout << "Cut5 : " << trimmer5 << " inserted : " << result.second << std::endl;
	result = Set.insert(trimmer6);
	std::cout << "Cut6 : " << trimmer6 << " inserted : " << result.second << std::endl;
	std::cout << "Set contains " << Set.size() << " elements " << std::endl;

	AllCutStorage map_set;
	map_set["a"].insert(trimmer1);
	map_set["a"].insert(trimmer2);
	map_set["a"].insert(trimmer3);
	map_set["b"].insert(trimmer4);
	map_set["b"].insert(trimmer5);
	map_set["b"].insert(trimmer6);

	std::cout << "Set 'a' contains " << map_set["a"].size() << " elements " << std::endl;
	std::cout << "Set 'b' contains " << map_set["a"].size() << " elements " << std::endl;

	return(0);
}