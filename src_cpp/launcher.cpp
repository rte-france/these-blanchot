#include "launcher.h"
#include "xprs.h"
#include "Benders.h"
#include "Timer.h"

#include "BendersOptions.h"


BendersOptions build_benders_options(int argc, char** argv) {
	BendersOptions result;
	if (argc == 4) {
		result.read(argv[3]);
		result.INPUTROOT = argv[1];
		std::string const summary_name(argv[2]);
		result.STRUCTURE_FILE = summary_name;
	}
	else if (argc == 2){
		result.read(argv[1]);
	}
	else if (argc == 3) {
		result.read(argv[2]);
		result.INPUTROOT = argv[1];
	}
	result.MASTER_NAME = result.INPUTROOT + PATH_SEPARATOR + result.MASTER_NAME;
	return result;
}

/*!
*  \brief Build the input from the structure file
*
*	Function to build the map linking each problem name to its variables and their id
*
*  \param root : root of the structure file
*
*  \param summary_name : name of the structure file
*
*  \param coupling_map : empty map to increment
*/
int build_input(BendersOptions const & options, CouplingMap & coupling_map) {
	coupling_map.clear();
	std::ifstream summary(options.STRUCTURE_FILE, std::ios::in);
	if (!summary) {
		std::cout << "Cannot open file summary " << options.STRUCTURE_FILE << std::endl;
		return 0;
	}
	std::string line;
	if (options.SLAVE_NUMBER == -1) {
		while (std::getline(summary, line))
		{
			std::stringstream buffer(line);
			std::string problem_name;
			std::string variable_name;
			int variable_id;
			buffer >> problem_name;
			problem_name = options.INPUTROOT + PATH_SEPARATOR + problem_name;
			buffer >> variable_name;
			buffer >> variable_id;
			coupling_map[problem_name].insert(std::pair<std::string, int>(variable_name, variable_id));
		}
	}
	else {
		int i(0);
		bool master_found(false);
		std::string problem_name;
		std::string variable_name;
		int variable_id;
		while (!(master_found) || (i <= options.SLAVE_NUMBER))
		{
			std::getline(summary, line);
			std::stringstream buffer(line);
			buffer >> problem_name;
			problem_name = options.INPUTROOT + PATH_SEPARATOR + problem_name;
			buffer >> variable_name;
			buffer >> variable_id;
			if (problem_name == options.MASTER_NAME) {
				coupling_map[problem_name].insert(std::pair<std::string, int>(variable_name, variable_id));
				master_found = true;
			}
			else if (coupling_map.find(problem_name) != coupling_map.end()) {
				coupling_map[problem_name].insert(std::pair<std::string, int>(variable_name, variable_id));
			}
			else {
				coupling_map[problem_name].insert(std::pair<std::string, int>(variable_name, variable_id));
				i++;
			}
		}
		coupling_map.erase(problem_name);
	}
	summary.close();
	return 0;
}


/*!
*  \brief Execute the Benders algorithm in sequential
*/
void sequential_launch(BendersOptions const & options) {
	Timer timer;
	XPRSinit("");
	CouplingMap input;
	build_input(options, input);
	Benders benders(input, options);
	options.print(std::cout);
	benders.run(std::cout);
	benders.free();
	XPRSfree();
	std::cout << "Problem ran in " << timer.elapsed() << " seconds" << std::endl;
}


enum Attribute {
	INT,
	INT_VECTOR,
	CHAR_VECTOR,
	DBL_VECTOR,
	MAX_ATTRIBUTE
};

enum IntAttribute {
	NROWS,
	NCOLS,
	NELES,
	MAX_INT_ATTRIBUTE
};


enum IntVectorAttribute {
	MSTART,
	MINDEX,
	MAX_INT_VECTOR_ATTRIBUTE,
};
enum CharVectorAttribute {
	ROWTYPE,
	COLTYPE,
	MAX_CHAR_VECTOR_ATTRIBUTE
};

enum DblVectorAttribute {
	MVALUE,
	RHS,
	RANGE,
	OBJ,
	LB,
	UB,
	MAX_DBL_VECTOR_ATTRIBUTE
};
typedef std::tuple<std::vector<int>, std::vector<IntVector>, std::vector<CharVector>, std::vector<DblVector> > raw_standard_lp_data;

class StandardLp {
private:
	// to be used in boost serialization for mpi transfer
	raw_standard_lp_data _data;
public:
	void init() {
		std::get<Attribute::INT>(_data).assign(IntAttribute::MAX_INT_ATTRIBUTE, 0);

		std::get<Attribute::INT_VECTOR>(_data).assign(IntVectorAttribute::MAX_INT_VECTOR_ATTRIBUTE, IntVector());
		std::get<Attribute::CHAR_VECTOR>(_data).assign(CharVectorAttribute::MAX_CHAR_VECTOR_ATTRIBUTE, CharVector());
		std::get<Attribute::DBL_VECTOR>(_data).assign(DblVectorAttribute::MAX_DBL_VECTOR_ATTRIBUTE, DblVector());
	}
	StandardLp(XPRSprob & _xp) {
		init();

		XPRSgetintattrib(_xp, XPRS_COLS, &std::get<Attribute::INT>(_data)[IntAttribute::NCOLS]);
		XPRSgetintattrib(_xp, XPRS_ROWS, &std::get<Attribute::INT>(_data)[IntAttribute::NROWS]);
		XPRSgetintattrib(_xp, XPRS_ELEMS, &std::get<Attribute::INT>(_data)[IntAttribute::NELES]);

		std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].assign(std::get<Attribute::INT>(_data)[IntAttribute::NROWS]+1, 0);
		std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].assign(std::get<Attribute::INT>(_data)[IntAttribute::NELES], 0);
		
		std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].assign(std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], 0);
		std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].assign(std::get<Attribute::INT>(_data)[IntAttribute::NROWS], 'E');
		
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].assign(std::get<Attribute::INT>(_data)[IntAttribute::NELES], 0);
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].assign(std::get<Attribute::INT>(_data)[IntAttribute::NROWS], 0);
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].assign(std::get<Attribute::INT>(_data)[IntAttribute::NROWS], 0);

		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].assign(std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], 0);
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].assign(std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], 0);
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].assign(std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], 0);

		int ncoeffs(0);

		XPRSgetrows(_xp, std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].data(), std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].data(), std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].data(), std::get<Attribute::INT>(_data)[IntAttribute::NELES], &ncoeffs, 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS]-1);
		
		XPRSgetrowtype(_xp, std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS] - 1);
		XPRSgetrhs(_xp, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS] - 1);
		XPRSgetrhsrange(_xp, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS] - 1);
		XPRSgetcoltype(_xp, std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS] - 1);
		XPRSgetlb(_xp, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS] - 1);
		XPRSgetub(_xp, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS] - 1);
		XPRSgetobj(_xp, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS] - 1);

	}

	int append_in(XPRSprob & xp) const{
		IntVector newmindex(std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX]);
		int ncols(0);
		XPRSgetintattrib(xp, XPRS_COLS, &ncols);

		// symply increment the columns indexes
		for (auto & i : newmindex) {
			i += ncols;
		}
		XPRSaddcols(xp, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], 0, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].data(), NULL, NULL, NULL, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].data(), std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].data());
		XPRSchgcoltype(xp, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], newmindex.data(), std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].data());
		XPRSaddrows(xp, std::get<Attribute::INT>(_data)[IntAttribute::NROWS], std::get<Attribute::INT>(_data)[IntAttribute::NELES], std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].data(), std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].data(), std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].data(), std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].data(), newmindex.data(), std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].data());
		return ncols;
	}
};

void merge_mps(BendersOptions const &options) {
	XPRSinit("");
	CouplingMap input;
	build_input(options, input);
	XPRSprob full;
	XPRScreateprob(&full);

	XPRSfree();
}

void usage(int argc) {
	if (argc < 2) {
		std::cout << "usage is : <exe> <root_dir> <structure_file> <option_file> " << std::endl;
		BendersOptions input;
		input.write_default();
		std::exit(0);
	}
	else {
		std::cout << "argc = " << argc << std::endl;
	}
}

