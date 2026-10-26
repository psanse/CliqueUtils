/**
 * @file main_clq.cpp
 * @brief Demo executable for the clique and independent set utilities in clq_func.h.
 *
 * @details With no arguments it runs on a small demo graph; otherwise it reads the graph
 * from the DIMACS file given as the first argument. Prints a greedy clique lower bound
 * and the ISEQ coloring upper bound for the maximum clique.
 *
 * @author pss
 */

#include <iostream>
#include "graph/graph.h"
#include "clique_utils.h"

using namespace bitgraph;

/**
 * @brief Prints clique lower bounds and the coloring upper bound of a graph
 * @param ug input graph
 */
static void print_bounds(const ugraph& ug) {

	ugraph::vertex_bitset_t bbsg{ ug.size(), true };		//all vertices

	std::vector<int> clq;
	clique_utils::find_clique_max_deg(ug, clq, bbsg);

	std::cout << "|V|=" << ug.num_vertices() << " |E|=" << ug.num_edges() << std::endl;
	std::cout << "clique LB (first-to-last greedy): " << clique_utils::find_clique_lb(ug) << std::endl;
	std::cout << "clique LB (max degree greedy):    " << clq.size() << " {";
	for (const auto& v : clq) { std::cout << " " << v; }
	std::cout << " }" << std::endl;
	std::cout << "clique UB (ISEQ coloring):        " << clique_utils::ISEQ(ug, bbsg) << std::endl;
}

/**
 * @brief Demo: clique bounds of a graph read from a file, or of a small demo graph
 * @param argc number of command line arguments
 * @param argv command line arguments; optional first argument: path to a DIMACS graph file
 * @returns 0 on success
 */
int main(int argc, char** argv) {

	if (argc > 1) {
		ugraph ug{ argv[1] };
		print_bounds(ug);
	}
	else {
		//triangle {0, 3, 4} plus edges 0-1 and 2-5
		ugraph ug{ 6 };
		ug.add_edge(0, 1);
		ug.add_edge(0, 3);
		ug.add_edge(0, 4);
		ug.add_edge(2, 5);
		ug.add_edge(3, 4);
		print_bounds(ug);			//maximum clique {0, 3, 4}
	}

	return 0;
}
