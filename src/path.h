#ifndef PATH_H
#define PATH_H
#include "gccheaders.h"
#include "mpi_detection.h"
#include <vector>
#include <iostream>

#define stack_el std::pair<basic_block, unsigned int>

// This object serves as a path unity checker
// It is initialised with a basic block, then used by calling the `common_path` method
class PathFinder {
	public:
		basic_block origin;
		PathFinder(basic_block bb);

		// This method is the core of the object's functionnality
		// it returns a boolean, set to true if all the paths emanating from
		// the basic block it was created with contains the same MPI collectives,
		// in the same sequence
		bool common_path();

		// The path that all graph searches must abide to.
		// It contains the codes of the collectives we must encounter, in order
		// It is built using the `sample_path` method
		std::vector<unsigned int> path;

		void state_objective();

	private:
		bool done;

		// This stack is the central element of the object, each element
		// containing a basic block and an index. While performing a DFS,
		// the index will be used to verify that if the bb contains a collective,
		// it is equal to the `index`th collective of the previously calculated path
		std::vector<stack_el> stack;

		// This methods performs a depth first 'plunge' of the graph,
		// beginning at `origin` and going depth first until the end is reached.
		// The encountered path is recorded and used as a base for calculations
		void sample_path();
};

#endif
