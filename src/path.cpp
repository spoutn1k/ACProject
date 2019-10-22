#include "path.h"
#include <diagnostic.h>
#define SAME_LOOP(bb1, bb2) (bb1->loop_father->num == bb2->loop_father->num)

PathFinder::PathFinder(basic_block bb) {
	this->origin = bb;
	this->path = std::vector<unsigned int>();

	this->stack = std::vector<stack_el>();
}

bool PathFinder::common_path() {
	// Returns false if in a loop
	// TODO be better than this
	if (origin->loop_father->num)
		return false;

	basic_block current;
	unsigned int index;
	int coll;
	edge arc;
	edge_iterator edge_iter;

	// First, perform a single run to get a path to compare to
	sample_path();

	// Initialize the DFS by inserting the origin node in the stack
	stack.push_back(stack_el(origin, 0));

	while (stack.size() > 0) {
		// std::vector.pop_back does not return an element
		// So we use this workaround instead
		stack_el popped = stack.back();
		stack.pop_back();

		// The bb to analyse
		current = popped.first;

		// The index of the collective we expect to find
		index = popped.second;

		// If the current bb contains a collective,
		// compare it to the expected value
		if ((coll = collective(current)) != -1) {
			// We found a path that is longer than expected
			// There are multiple paths
			if (index == path.size())
				return false;
			// If it is different, then all paths are not equal
			// ABORT and return false
			if (coll != (int) path[index])
				return false;

			// If it is, just mark that we expect the next collective
			index++;
		}

		// If the current bb is the header of a loop,
		// we do not want to go down into it
		if (!SAME_LOOP(origin, current)) {
			// Instead, we find all the successors that belong to the same
			// 'loop space' as the origin, and push them on the stack
			FOR_EACH_EDGE (arc, edge_iter, current->succs) {
				if (SAME_LOOP(origin, arc->dest))
					stack.push_back(stack_el(arc->dest, index));
			}
		} else {
			// If it is not from a loop, we push all of its successors to be evaluated
			FOR_EACH_EDGE (arc, edge_iter, current->succs)
				stack.push_back(stack_el(arc->dest, index));

			// If we arrive at the end of the graph, and the whole path has not been discovered
			if (!EDGE_COUNT(current->succs) && index != path.size())
				return false;
		}
		// NOTE it is important to test before iterating on the edges
		// as not adding a loop's header would impair the search
	}

	// Once the DFS has ended, if no path divergence has been found, we clear the bb
	return true;
}


void PathFinder::sample_path() {
	// Returns false if in a loop
	// TODO be better than this
	if (origin->loop_father->num)
		return;

	basic_block current = this->origin, next;
	int coll;
	edge arc;
	edge_iterator edge_iterator;

	// Continue until the end is reached
	while (EDGE_COUNT(current->succs)) {
		// If there is a collective, record it
		if ((coll = collective(current)) != -1)
			this->path.push_back(coll);

		next = EDGE_SUCC(current, 0)->dest;

		// If next is not on the same loop, get his first successor that is
		if (!SAME_LOOP(current, next)) {
			FOR_EACH_EDGE (arc, edge_iterator, next->succs) {
				if (SAME_LOOP(arc->dest, current)) {
					next = arc->dest;
					continue;
				}
			}
		}

		current = next;
	}
}

void PathFinder::state_objective() {
	this->sample_path();
	std::cout << origin->index << ": ";

	for (int node: this->path)
		std::cout << node << " ";

	std::cout << "\n";
}
