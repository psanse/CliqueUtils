/**
 * @file clique_utils.h
 * @brief Utility functions and algorithms for cliques and independent sets.
 *
 * @details Provides common graph operations and heuristics related to
 *          clique and independent-set computation.
 *
 *          Originally developed in the COPT repository and later
 *          integrated into the standalone CliqueUtils library.
 *
 *          Requires C++14. C++17 features such as if constexpr
 *          are intentionally not used.
 *
 * @author pss
 * @date 2019-07-10
 * @note Imported into CliqueUtils on 2025-05-09.
 * @note Last updated: 2026-10-26.
 */

#ifndef CLIQUE_UTILS_H
#define CLIQUE_UTILS_H

#include <iostream>
#include "utils/logger.h"
#include "bitscan/bbobject.h"		
#include "bitscan/bitblock.h"		//for bitblock operations
#include "bitscan/bbset.h"
#include "bitscan/bbscan_view.h"	//independent scan views
#include "graph/graph_types.h"

#include <cassert>					//DEBUG run-time assertions 

namespace clique_utils {

	using vertex_pos_t = unsigned int;		//position type in a vertex collection

	//////////////////
	// heuristics

	/**
	 * @brief Returns the number of consecutive vertices that form
	 *        an independent set, starting at a given position.
	 *
	 * @details Vertices are examined in their given order.
	 *          The function stops at the first vertex adjacent
	 *          to any previously examined vertex.
	 *
	 * @tparam GraphT Graph type.
	 * @param graph Input graph.
	 * @param vertices Array containing the vertex sequence.
	 * @param begin First position to examine (inclusive).
	 * @param end Last position to examine (exclusive).
	 * @return Number of vertices in the independent prefix.
	 *
	 * @pre begin <= end.
	 * @pre vertices contains at least end valid entries.
	 */
	template<class GraphT>
	inline
		int iset_prefix_size(
			const GraphT& graph,
			const vertex_t vertices[],
			vertex_pos_t begin,
			vertex_pos_t end)
	{
		assert(begin <= end);

		for (vertex_pos_t i = begin; i < end; ++i) {
			for (vertex_pos_t j = begin; j < i; ++j) {
				if (graph.is_edge(vertices[j], vertices[i])) {
					return static_cast<int>(i - begin);
				}
			}
		}

		return static_cast<int>(end - begin);		//all vertices in the range form an independent set
	}

	/**
	 * @brief Computes the number of consecutive vertices that can extend
	 *        the singleton independent set {v}.
	 *
	 * @details Examines vertices sequentially in the half-open range
	 *          [begin, end), stopping at the first vertex adjacent to
	 *          v or to any previously accepted vertex.
	 *
	 *          Runs in O(n^2) worst-case time, where n = end - begin.
	 *
	 * @param graph Input graph.
	 * @param v Initial vertex of the independent set.
	 * @param vertices Array of candidate vertices.
	 * @param begin First position to examine (inclusive).
	 * @param end Last position to examine (exclusive).
	 * @return Number of accepted vertices, excluding v.
	 *
	 * @pre begin <= end.
	 * @pre vertices contains at least end elements.
	 * @pre v does not occur in vertices[begin, end).
	 */
	template<class GraphT>
	inline
		int iset_prefix_size(
			const GraphT& g,
			vertex_t v,
			const vertex_t vertices[],
			vertex_pos_t begin,
			vertex_pos_t end)
	{
		assert(begin <= end);

		for (vertex_pos_t i = begin; i < end; ++i) {

			const vertex_t w = vertices[i];

			// Check adjacency to the initial vertex.
			if (g.neighbors(v).is_bit(w)) {
				return static_cast<int>(i - begin);
			}

			const auto& neighbors = g.neighbors(w);

			// Check adjacency to previously accepted vertices.
			for (vertex_pos_t j = begin; j < i; ++j) {
				if (neighbors.is_bit(vertices[j])) {
					return static_cast<int>(i - begin);
				}
			}
		}

		return static_cast<int>(end - begin);
	}

	/**
	 * @brief Greedily extends an existing clique using a sequence of candidate vertices.
	 *           The resulting clique is maximal with respect to the examined
	 *           candidates, but is not necessarily a maximum clique.
	 *
	 * @details Examines vertices sequentially in the half-open range
	 *          [begin, end). Each vertex is added to @p clq if it is
	 *          adjacent to all vertices currently in the clique.
	 *          Incompatible vertices are skipped.
	 *
	 *          Runs in O(n(m + n)) worst-case time, where
	 *          n = end - begin and m is the initial size of @p clq.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @param graph Input graph.
	 * @param clq Input clique, extended in place.
	 * @param vertices Array of candidate vertices.
	 * @param begin First position to examine (inclusive).
	 * @param end Last position to examine (exclusive).
	 * @return Number of vertices added to @p clq.
	 *
	 * @pre begin <= end.
	 * @pre vertices contains at least end elements.
	 * @pre clq is a valid clique.
	 * @pre Candidate vertices are distinct and do not occur in clq.
	 */
	template<class GraphT>
	inline
		int extend_clique(
			const GraphT& graph,
			bitgraph::vertices& clq,
			const vertex_t vertices[],
			vertex_pos_t begin,
			vertex_pos_t end)
	{
		assert(begin <= end);

		const auto initial_size = clq.size();

		for (vertex_pos_t i = begin; i < end; ++i) {

			const vertex_t w = vertices[i];
			bool compatible = true;

			// Check adjacency to all vertices in the current clique.
			for (const vertex_t v : clq) {
				if (!graph.is_edge(v, w)) {
					compatible = false;
					break;
				}
			}

			if (compatible) {
				clq.push_back(w);
			}
		}

		return static_cast<int>(clq.size() - initial_size);
	}

	/**
	 * @brief Greedily constructs a clique from a set of candidate vertices.
	 *
	 * @details Vertices are selected in increasing or decreasing order,
	 *          according to @p Reverse. After selecting each vertex,
	 *          the remaining candidates are restricted to its neighbors.
	 *
	 *          The resulting clique is maximal with respect to the
	 *          initial candidate set, but is not necessarily maximum.
	 *
	 *          The input bitset is not modified.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @tparam Reverse If true, processes vertices in decreasing order;
	 *                 otherwise, in increasing order.
	 *
	 * @param graph Input graph.
	 * @param clq Output clique, cleared before construction.
	 * @param vertices Bitset of candidate vertices.
	 * @return Number of vertices in the resulting clique.
	 *
	 * @note Uses bitset intersection to efficiently prune candidates.
	 */
	template<class GraphT, bool Reverse = false>
	inline
		int find_clique(
			const GraphT& graph,
			bitgraph::vertices& clq,
			const typename GraphT::vertex_bitset_t& vertices)
	{

		typename GraphT::vertex_bitset_t candidates{ vertices };
		clq.clear();

		candidates.init_scan(
			Reverse ? bbo::NON_DESTRUCTIVE_REVERSE : bbo::NON_DESTRUCTIVE
		);

		while (true) {

			const vertex_t v = Reverse 
					? candidates.prev_bit():
					  candidates.next_bit();

			if (v == bbo::noBit) {
				break;
			}

			clq.push_back(v);

			// Restrict remaining candidates to neighbors of v.
			if (Reverse) {
				for (block_index_t nBB = WDIV(v); nBB >= 0; --nBB) {
					candidates.block(nBB) &= graph.neighbors(v).block(nBB);
				}
			}
			else {
				for (block_index_t nBB = WDIV(v);
					nBB < static_cast<int>(graph.num_blocks());
					++nBB)
				{
					candidates.block(nBB) &= graph.neighbors(v).block(nBB);
				}
			}
		}

		return static_cast<int>(clq.size());
	}

	/**
	 * @brief Greedily constructs a clique using a maximum-degree heuristic.
	 *
	 * @details At each iteration, selects a vertex with maximum degree
	 *          in the subgraph induced by the current candidates.
	 *          The candidate set is then restricted to its neighbors.
	 *
	 *          Ties are resolved according to @p Reverse.
	 *          When no candidate has a neighbor, one remaining vertex
	 *          is added to the clique and the algorithm terminates.
	 *
	 *          The resulting clique is maximal with respect to the
	 *          initial candidate set, but is not necessarily maximum.
	 *          The input bitset is not modified.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @tparam Reverse If true, breaks ties in favor of higher vertex IDs;
	 *                 otherwise, in favor of lower vertex IDs.
	 *
	 * @param graph Input graph.
	 * @param clq Output clique, cleared before construction.
	 * @param vertices Bitset of candidate vertices.
	 * @return Number of vertices in the resulting clique.
	 *
	 * @note Uses population counts to compute candidate degrees.
	 */
	template<class GraphT, bool Reverse = false>
	inline int find_clique_max_deg(
		const GraphT& graph,
		bitgraph::vertices& clq,
		const typename GraphT::vertex_bitset_t& vertices)
	{
		typename GraphT::vertex_bitset_t candidates{ vertices };
		clq.clear();

		while (!candidates.is_empty()) {

			degree_t max_degree = -1;
			vertex_t best_vertex = bbo::noBit;

			candidates.init_scan(
				Reverse ? bbo::NON_DESTRUCTIVE_REVERSE : bbo::NON_DESTRUCTIVE
			);

			
			// Find a maximum-degree vertex in the candidate subgraph.
			while (true) {

				const vertex_t v = Reverse
					? candidates.prev_bit()
					: candidates.next_bit();

				if (v == bbo::noBit) {
					break;
				}

				int degree = 0;
				const auto& neighbors = graph.neighbors(v);

				for (block_index_t nBB = 0; nBB < graph.num_blocks(); ++nBB) {
					degree += bitgraph::bblock::popc64(
						candidates.block(nBB) & neighbors.block(nBB));
				}

				// Strict comparison preserves the first vertex in a tie.
				if (degree > max_degree) {
					max_degree = degree;
					best_vertex = v;
				}
			}

			// Select the best candidate.
			clq.push_back(best_vertex);

			// Restrict candidates to neighbors of the selected vertex.
			candidates &= graph.neighbors(best_vertex);
		}

		return static_cast<int>(clq.size());
	}

	/**
	 * @brief Finds the largest clique among those constructed greedily
	 *        from each candidate vertex.
	 *
	 * @details Each vertex in @p vertices is used as the initial vertex
	 *          of a clique. The clique is greedily extended with subsequent
	 *          candidate vertices in increasing order, using bitset
	 *          intersections to maintain adjacency to all selected vertices.
	 *
	 *          The largest clique found is returned in @p clq.
	 *          The result is not necessarily a maximum clique.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @param graph Input graph.
	 * @param clq Output clique, cleared before construction.
	 * @param vertices Bitset of candidate vertices.
	 * @return Number of vertices in the resulting clique.
	 *
	 * @note The input bitset is not modified.
	 */
	template<class GraphT>
	inline int find_clique_from_pool(
		const GraphT& graph,
		bitgraph::vertices& clq,
		const typename GraphT::vertex_bitset_t& vertices)
	{
		using bitset_t = typename GraphT::vertex_bitset_t;

		clq.clear();

		bitset_t candidates(vertices);

		bitgraph::vertices clq_curr;

		// Independent non-destructive scan of the (const) input: no copy needed.
		bitgraph::view::ForwardView<const bitset_t> seeds(vertices);

		vertex_t v = bbo::noBit;

		while ((v = seeds.next_bit()) != bbo::noBit) {

			clq_curr.clear();
			clq_curr.push_back(v);

			// Candidates adjacent to v, with IDs greater than v.
			bitgraph::AND<true>(
				v,
				graph.num_vertices() - 1,
				graph.neighbors(v),
				vertices,
				candidates
			);

			candidates.init_scan(bbo::DESTRUCTIVE);

			vertex_t w = bbo::noBit;

			while ((w = candidates.next_bit_del()) != bbo::noBit) {

				clq_curr.push_back(w);

				// Restrict remaining candidates to neighbors of w.
				const auto& neighbors_w = graph.neighbors(w);

				for (block_index_t nBB = BITSCAN_WDIV(w);
					nBB < graph.num_blocks();
					++nBB)
				{
					candidates.block(nBB) &= neighbors_w.block(nBB);
				}
			}

			// Retain the largest clique found.
			if (clq_curr.size() > clq.size()) {
				clq = clq_curr;
			}
		}

		return static_cast<int>(clq.size());
	}
		
	/////////////////
	//boolean operations

	/**
	 * @brief Checks whether a non-empty set of vertices is an independent set.
	 *
	 * @details A set is independent if no two of its vertices are adjacent.
	 *          The empty set is not considered independent.
	 *
	 *          Uses bitset operations to detect edges between selected vertices.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @param graph Input graph.
	 * @param vertices Bitset containing the vertices to examine.
	 * @return True if @p vertices is a non-empty independent set;
	 *         false otherwise.
	 *
	 * @pre @p vertices has capacity for at least graph.num_vertices() bits.
	 *
	 * @note The input bitset is not modified.
	 * @todo Optimize for sparse graph types.
	 */
	template<class GraphT>
	inline
		bool is_iset(
			const GraphT& graph,
			const typename GraphT::vertex_bitset_t& vertices)
	{
		using bitset_t = typename GraphT::vertex_bitset_t;

		if (vertices.is_empty()) {
			return false;
		}

		// Independent non-destructive scan of the (const) input.
		bitgraph::view::ForwardView<const bitset_t> scan(vertices);
		vertex_t v = bbo::noBit;
		while ((v = scan.next_bit()) != bbo::noBit) {

			//check neighborhood of v
			const auto& neighbors_v = graph.neighbors(v);

			// Check only blocks from the current vertex onwards.
			for (block_index_t nBB = BITSCAN_WDIV(v);
				nBB < graph.num_blocks(); ++nBB) {
							
				if (vertices.block(nBB) & neighbors_v.block(nBB)) {
					return false;
				}
				
			}

		}//end bitscan

		return true;  // iset confirmed
	}

	/**
	 * @brief Checks whether a non-empty set of vertices is an independent set.
	 *
	 * @details A set is independent if no two of its vertices are adjacent.
	 *          The empty set is not considered independent.
	 *
	 *          Runs in O(n^2) worst-case time, where n is the number
	 *          of vertices in the input set.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @param graph Input graph.
	 * @param vertices List of vertices to examine.
	 * @return True if @p vertices is a non-empty independent set;
	 *         false otherwise.
	 *
	 * @pre All vertex identifiers are valid and distinct.
	 *
	 * @note The input vertex list is not modified.
	 */
	template<class GraphT>
	inline
		bool is_iset(
			const GraphT& graph,
			const bitgraph::vertices& vertices)
	{

		const std::size_t vertex_count = vertices.size();

		if (vertex_count == 0) {
			return false;
		}

		for (std::size_t i = 0; i + 1 < vertex_count; ++i) {
			for (std::size_t j = i + 1; j < vertex_count; ++j) {

				if (graph.is_edge(vertices[i], vertices[j])) {
					return false;
				}
			}
		}

		return true;		
	}

	/**
	 * @brief Checks whether a vertex can extend an independent set.
	 *
	 * @details Returns true if @p v is non-adjacent to every vertex
	 *          in @p vertices. An empty set can always be extended.
	 *
	 *          Runs in O(n) worst-case time, where n is the number
	 *          of vertices in the input set.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @param graph Input graph.
	 * @param vertices Independent set to be extended
	 *			(empty set can always be extended).
	 * @param v Candidate vertex.
	 * @return True if @p v can extend the independent set;
	 *         false otherwise.
	 *
	 * @pre All vertex identifiers are valid.
	 * @pre vertices is an independent set, possibly empty.
	 * @pre v does not occur in vertices.
	 */
	template<class GraphT>
	inline
		bool can_extend_iset(
			const GraphT& graph, 
			const bitgraph::vertices& vertices, 
			vertex_t v)
	{
		for (const vertex_t w : vertices) {
			if (graph.is_edge(v, w)) {
				return false;
			}
		}

		return true;
	}


	/**
	 * @brief Checks whether a non-empty set of vertices is a clique.
	 *
	 * @details A set is a clique if every pair of its vertices is adjacent.
	 *          The empty set is not considered a clique.
	 *
	 *          Uses population counts: each vertex must be adjacent to
	 *          all the other vertices in the set.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @param graph Input graph.
	 * @param vertices Bitset containing the vertices to examine.
	 * @return True if @p vertices is a non-empty clique; false otherwise.
	 *
	 * @note The input bitset is not modified.
	 * @todo Optimize for sparse graph types.
	 */
	template<class GraphT>
	inline
		bool is_clique(
			const GraphT& graph,
			const typename GraphT::vertex_bitset_t& vertices)
	{
		using bitset_t = typename GraphT::vertex_bitset_t;

		const int pc = vertices.count();
		if (pc == 0) {
			return false;
		}

		// Independent non-destructive scan of the (const) input.
		bitgraph::view::ForwardView<const bitset_t> scan(vertices);
		vertex_t v = bbo::noBit;
		while ((v = scan.next_bit()) != bbo::noBit) {

			// v must be adjacent to the other (pc - 1) vertices.
			const auto& neighbors_v = graph.neighbors(v);
			int common = 0;
			for (block_index_t nBB = 0; nBB < graph.num_blocks(); ++nBB) {
				common += bitgraph::bblock::popc64(
					vertices.block(nBB) & neighbors_v.block(nBB));
			}

			if (common != pc - 1) {
				return false;
			}
		}

		return true;
	}

	/**
	 * @brief Checks whether a non-empty array of vertices is a clique.
	 *
	 * @details Runs in O(n^2) worst-case time, where n = size.
	 *          The empty set is not considered a clique.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @param graph Input graph.
	 * @param vertices Array of vertices to examine.
	 * @param size Number of vertices in @p vertices.
	 * @return True if the vertices form a non-empty clique; false otherwise.
	 *
	 * @pre All vertex identifiers are valid and distinct.
	 */
	template<class GraphT>
	inline
		bool is_clique(
			const GraphT& graph,
			const vertex_t vertices[],
			std::size_t size)
	{
		if (size == 0) {
			return false;
		}

		for (std::size_t i = 0; i + 1 < size; ++i) {
			for (std::size_t j = i + 1; j < size; ++j) {
				if (!graph.is_edge(vertices[i], vertices[j])) {
					return false;
				}
			}
		}

		return true;
	}

	/**
	 * @brief Checks whether a non-empty list of vertices is a clique.
	 *
	 * @details Runs in O(n^2) worst-case time, where n is the number
	 *          of vertices. The empty set is not considered a clique.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @param graph Input graph.
	 * @param vertices List of vertices to examine.
	 * @return True if @p vertices is a non-empty clique; false otherwise.
	 *
	 * @pre All vertex identifiers are valid and distinct.
	 */
	template<class GraphT>
	inline
		bool is_clique(
			const GraphT& graph,
			const bitgraph::vertices& vertices)
	{
		return is_clique(graph, vertices.data(), vertices.size());
	}

	/**
	 * @brief Checks whether a vertex is adjacent to every vertex in a list.
	 *
	 * @details If @p vertices is a clique, the result tells whether
	 *          {vertices + v} is also a clique. An empty list yields true.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @param graph Input graph.
	 * @param vertices List of vertices, possibly empty.
	 * @param v Candidate vertex.
	 * @return True if @p v is adjacent to all vertices in @p vertices
	 *         (or @p vertices is empty); false otherwise.
	 *
	 * @pre v does not occur in vertices.
	 */
	template<class GraphT>
	inline
		bool is_clique(
			const GraphT& graph,
			const bitgraph::vertices& vertices,
			vertex_t v)
	{
		for (const vertex_t w : vertices) {
			if (!graph.is_edge(v, w)) {
				return false;
			}
		}

		return true;
	}

	//////////////////////
	// maximum clique bounds

	/**
	 * @brief Computes a lower bound for the maximum clique in graph[vertices].
	 *
	 * @details Equivalent to find_clique() but does not store the clique.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @tparam Reverse If true, vertices are taken from last to first;
	 *                 otherwise, from first to last.
	 * @param graph Input graph.
	 * @param vertices Bitset of candidate vertices.
	 * @return Size of the greedy clique.
	 *
	 * @note The input bitset is not modified.
	 */
	template<class GraphT, bool Reverse = false>
	inline
		int find_clique_lb(
			const GraphT& graph,
			const typename GraphT::vertex_bitset_t& vertices)
	{
		typename GraphT::vertex_bitset_t candidates{ vertices };
		int lb = 0;

		candidates.init_scan(
			Reverse ? bbo::NON_DESTRUCTIVE_REVERSE : bbo::NON_DESTRUCTIVE
		);

		while (true) {

			const vertex_t v = Reverse
				? candidates.prev_bit()
				: candidates.next_bit();

			if (v == bbo::noBit) {
				break;
			}

			++lb;

			// Restrict remaining candidates to neighbors of v.
			const auto& neighbors_v = graph.neighbors(v);

			if (Reverse) {
				for (block_index_t nBB = WDIV(v); nBB >= 0; --nBB) {
					candidates.block(nBB) &= neighbors_v.block(nBB);
				}
			}
			else {
				for (block_index_t nBB = WDIV(v);
					nBB < static_cast<int>(graph.num_blocks());
					++nBB)
				{
					candidates.block(nBB) &= neighbors_v.block(nBB);
				}
			}
		}

		return lb;
	}

	/**
	 * @brief Computes a lower bound for the maximum clique in the whole graph.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @tparam Reverse If true, vertices are taken from last to first;
	 *                 otherwise, from first to last.
	 * @param graph Input graph.
	 * @return Size of the greedy clique.
	 */
	template<class GraphT, bool Reverse = false>
	inline
		int find_clique_lb(const GraphT& graph)
	{
		const typename GraphT::vertex_bitset_t all_vertices(
			static_cast<int>(graph.size()), true);

		return find_clique_lb<GraphT, Reverse>(graph, all_vertices);
	}

	/////////////////////
	// maximum clique upper bound (SEQ vertex coloring)

	namespace detail {

		/**
		 * @brief Greedy sequential independent-set coloring.
		 *
		 * @details Shared implementation of the ISEQ overloads. @p color_of
		 *          is called as color_of(v, color) for every colored vertex.
		 */
		template<class GraphT, class ColorSink>
		inline
			int iseq_color(
				const GraphT& graph,
				const typename GraphT::vertex_bitset_t& vertices,
				ColorSink color_of)
		{
			int pending = vertices.count();
			if (pending == 0) {
				return 0;
			}

			int color = 1;
			vertex_t v = bbo::noBit;

			typename GraphT::vertex_bitset_t uncolored(vertices);
			typename GraphT::vertex_bitset_t candidates(graph.size());

			while (true) {

				// Remaining vertices to be colored.
				candidates = uncolored;

				// Build a new independent set from the candidates.
				candidates.init_scan(bbo::DESTRUCTIVE);
				while ((v = candidates.next_bit_del(uncolored)) != bbo::noBit) {

					color_of(v, color);

					if (--pending == 0) {
						return color;		// all vertices colored
					}

					// Remove neighbors of v from the candidates.
					candidates.erase_block(WDIV(v), -1, graph.neighbors(v));
				}

				++color;
			}
		}

	}//end of namespace detail

	/**
	 * @brief Greedy sequential independent-set coloring of a vertex set.
	 *
	 * @details The number of colors is an upper bound for the maximum
	 *          clique in graph[vertices].
	 *
	 *          Runs in O(n^3) worst-case time, but is fast in practice
	 *          thanks to bit-parallelism.
	 *
	 * @tparam GraphT Undirected graph type.
	 * @param graph Input graph.
	 * @param vertices Bitset of vertices to be colored.
	 * @param colors (Optional) array of size graph.size() receiving the
	 *               color (starting at 1) of each colored vertex.
	 * @return Number of colors used.
	 */
	template<class GraphT>
	inline
		int ISEQ(
			const GraphT& graph,
			const typename GraphT::vertex_bitset_t& vertices,
			int* colors = nullptr)
	{
		return detail::iseq_color(graph, vertices,
			[colors](vertex_t v, int color) {
				if (colors) { colors[v] = color; }
			});
	}

	/**
	 * @brief Greedy sequential independent-set coloring of a vertex set.
	 *
	 * @param graph Input graph.
	 * @param vertices Bitset of vertices to be colored.
	 * @param colors Output vector of size graph.size() with the color
	 *               (starting at 1) of each colored vertex, 0 otherwise.
	 *               Left untouched if @p vertices is empty.
	 * @return Number of colors used.
	 */
	template<class GraphT>
	inline
		int ISEQ(
			const GraphT& graph,
			const typename GraphT::vertex_bitset_t& vertices,
			std::vector<int>& colors)
	{
		if (vertices.is_empty()) {
			return 0;
		}

		colors.assign(graph.size(), 0);

		return detail::iseq_color(graph, vertices,
			[&colors](vertex_t v, int color) { colors[v] = color; });
	}


}//end of namespace clique_utils


#endif // CLIQUE_UTILS_H




	//////////////////////////////////
	// VERY CLIQUE SPECIFIC - PLACE IT ELSEWHERE

	//// enlarges @clq with vertices in lv, starting from @begin
	//// can detect cliques by swapping one of the vertices in @quasi
	////
	//// RETURN val:number of vertices added to clq (0 or 1)
	////
	//// COMMENTS: format issues-MUST BE! @quasi[VERTEX]:=-1 or VERTEX

	//template<class GraphT>
	//int find_clq(GraphT& g, Vertices& clq, int* quasi, int begin, int* lv, int end) {
	//			
	//	int num_added = 0, counter = 0, iqv = -1;	

	//	//main loop - iterates over each vertex in the range
	//	for (int i = (end - 1); i >= begin ; i--){						//reverse direction! perhaps better for brock? (26/01/2021)
	//	//for (int i = begin; i < end; i++) {	
	//	// 	
	//		counter = 0;

	//		for (auto j = 0; j < clq.size(); ++j) {
	//			if (!g.is_edge(lv[i], clq[j])) { 
	//				counter++;				
	//				if (quasi[clq[j]] == -1 || lv[i] == quasi[clq[j]] || counter >= 2) { 
	//					counter = 2;  
	//					break; 
	//				}
	//				else { iqv = j;  };					
	//			}
	//		}
	//		
	//		if (counter == 0) {	
	//			LOG_ERROR("warning!- improving clique with previous dolls, find_clq(...)" << lv[i]);
	//			cin.get();
	//			clq.push_back(lv[i]);
	//			num_added++;
	//			break;						//quasi info may not be valid	
	//		}else if (counter == 1) {
	//			int qswap = quasi[clq[iqv]];
	//			if (  g.is_edge(lv[i], qswap) ) {	//quasi-clique check
	//				
	//				clq.push_back(lv[i]);
	//				clq[iqv] = qswap;			//swap vertex in @clq with the new quasi vertex 					
	//				num_added++;
	//															
	//				break;						//EXIT-quasi clique info is not valid any more
	//			}
	//			else {
	//				//LOG_INFO("ENCONTRADO SWAP SIN MEJORA: "<< lv[i]);
	//			}
	//			
	//		}			
	//	}

	//	return num_added;
	//}
	//	


	//////////////////////////////////
	// Incremental bounds VERY CLIQUE SPECIFIC - PLACE IT IN COPT

	//	template<class GraphT>
	//	inline
	//		void incUB(const GraphT& g, int ub[], size_t size) {
	//		//computes incremental clique bound for nodes 1 to size for the given graph
	//		ub[0] = 1;
	//		int max = 0;
	//		for (int v = 1; v < size; v++) {
	//			max = 0;
	//			for (int w = 0; w < v; w++) {
	//				if (g.is_edge(v, w)) {
	//					if (ub[w] > max) max = ub[w];
	//				}
	//			}
	//			ub[v] = max + 1;
	//		}
	//	}
	//
	//	template<class GraphT>
	//	inline
	//		int incUB(const GraphT& g, typename GraphT::vertex_bitset_t& bbsg, int ub[]) {
	//		// computes incremental clique bound for nodes in bbsg
	//		// RETURNS -1 if bbsg is empty, 0 in any other case
	//		// 
	//		// COMMENTS
	//		// Does not overwrite other values of ub[] 
	//		//
	//		//*EXPERIMENTAL*
	//
	//		Vertices nodes;
	//		bbsg.to_vector(nodes);			/* conversion-seems the simplest  thing to do */
	//		if (nodes.empty()) return -1;
	//		ub[nodes.front()] = 1;
	//
	//		int max = 0;
	//		for (int i = 1; i < nodes.size(); i++) {
	//			max = 0;
	//			for (int j = 0; j < i; j++) {
	//				if (g.is_edge(nodes[i], nodes[j])) {
	//					if (ub[nodes[j]] > max) max = ub[nodes[j]];
	//				}
	//			}
	//			ub[nodes[i]] = max + 1;
	//		}
	//		return 0;
	//	}


	////////////////////////
	//Others - outgoing / incoming edges

	//	template<class GraphT>
	//	inline
	//	int number_outgoing_edges(GraphT& g, std::vector<int> clq) {
	//		///////////////////////
	//		// RETURNS edges outgoing from every vertex of @clq
	//	
	//		int nb_edges = 0;
	//		typename GraphT::vertex_bitset_t bbcov(clq, g.number_of_vertices());
	//		for (int i = 0; i < clq.size(); i++) {
	//			for (int NBB = 0; NBB < g.num_blocks(); NBB++) {
	//				BITBOARD bb = g.get_neighbors(clq[i]).get_bitboard(NBB) &~bbcov.get_bitboard(NBB);
	//				nb_edges += BitBoard::popc64(bb);
	//			}
	//		}		
	//		return nb_edges;
	//	}
	//
	//	template<class GraphT>
	//	inline
	//		int max_number_outgoing_edges(GraphT& g, std::vector<vector<int>>& part, int& pos) {
	//		///////////////////////
	//		// RETURNS index of element in @part with maximum number of outgoing edges (-1 if empty)
	//
	//		int max = 0, num = 0;
	//		pos = -1;
	//		for (int i = 0; i < part.size(); i++) {
	//			num = number_outgoing_edges(g, part[i]);
	//			if (max < num) { max = num;  pos = i; }
	//		}
	//		return max;
	//	}
	//
	//	template<class GraphT>
	//	inline
	//		int min_number_outgoing_edges(GraphT& g, std::vector<vector<int>>& part, int& pos) {
	//		///////////////////////
	//		// RETURNS index of element in @part with minimum number of outgoing edges (-1 if empty)
	//
	//		int min = CLQ_MAXINT, num = 0;
	//		pos = -1;
	//		for (int i = 0; i < part.size(); i++) {
	//			num = number_outgoing_edges(g, part[i]);
	//			if (min > num) { min = num;  pos = i; }
	//		}
	//		return min;
	//	}

//////////////////////////////////
	// VERY CLIQUE SPECIFIC - PLACE IT IN COPT
	//	template<class GraphT>
	//	int quasi_clq(GraphT& g, Vertices& clq, int* quasi, typename GraphT::vertex_bitset_t& bbv) {
	//		////////////////////////////////
	//		// caches quasicliques in @quasi for consecutive vertices not in @clq (candidate quasi_vertices range [0-highest vertex index of clq[)
	//		// quasi[VERTEX_IN_CLQ]:= qv (@clq - {VERTEX_IN_CLQ} + {qv} is a clique in @g of the same size)
	//		// @bbv-set of vertices not considered in qv
	//		//
	//		// COMMENTS: quasi MUST be of size the number of vertices of @g
	//		//
	//		// RETURN val:number of quasi-cliques (-1 if a new clique is found)	
	//
	//		//ASSERT
	//		if (clq.empty()) return 0;
	//
	//		typename GraphT::vertex_bitset_t bbclq(clq, g.number_of_vertices());
	//		int num_added = 0, num_no_clq = 0, qv = -1, lastOfclq = bbclq.msbn64();
	//
	//		for (int v = 0; v < lastOfclq; v++) {									/* last vertex in clq excluded, since it MUST BE in @bbv*/
	//			if (bbclq.is_bit(v) || !bbv.is_bit(v)) continue;					//filters vertices in @clq and in @bbv
	//			num_no_clq = 0;
	//			for (int j = 0; j < clq.size(); j++) {
	//				if (!g.is_edge(v, clq[j])) {
	//					num_no_clq++;
	//					if (num_no_clq >= 2) { break; }
	//					else { qv = clq[j]; }
	//				}
	//			}
	//
	//			if (num_no_clq == 0) { //Already a clique and HAS NOT BEEN FOUND!
	//				clq.push_back(v);
	//				num_added++;
	//
	//#ifdef my_assert_qfunc					
	//				LOG_ERROR("clique found in preceding dolls-utils::quasi_clq()");
	//				LOG_ERROR("may only happen in QUASI-CLIQUE mode, and if a new solution of size > +1 has been found previously");
	//				/*if (is_clique(g, clq)) {
	//					LOG_ERROR("bizarre clique, exiting....-utils::quasi_clq()");
	//					LOG_ERROR("v that may be added:" << v);
	//					stringstream sstr;
	//					utils::stl::print_collection<Vertices>(clq, sstr);
	//					LOG_ERROR("clq with v included" << sstr.str());
	//					exit(-1);
	//				}
	//				else {
	//					LOG_ERROR("bizarre clique, exiting....-utils::quasi_clq()");
	//				}*/
	//#endif
	//				return -1;
	//			}
	//			else if (num_no_clq == 1) {
	//				quasi[qv] = v;
	//				num_added++;
	//			}
	//		}
	//		return num_added;
	//	}
	//


