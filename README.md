<p align="center">
  <img alt="C++14" src="https://img.shields.io/badge/C%2B%2B-14-00599C?logo=c%2B%2B&logoColor=white">
  <img alt="CMake" src="https://img.shields.io/badge/build-CMake%20%E2%89%A5%203.14-064F8C?logo=cmake&logoColor=white">
  <img alt="BitGraph" src="https://img.shields.io/badge/BitGraph-v2.0.0-informational">
</p>

# CliqueUtils: clique and independent set utilities for undirected graphs

Header-only C++ functions for cliques and independent sets: greedy heuristics, membership checks, a lower bound for the maximum clique and an upper bound from greedy sequential coloring. They are built on the bitset-based [BitGraph](https://github.com/psanse/BitGraph) library and are templated on the graph type.

> **Status:** work in progress. The unit tests pass with BitGraph v2.0.0; see [Known issues](#known-issues).

## Usage

Include [`src/clique_utils.h`](src/clique_utils.h). The functions live in the namespace `clique_utils` and work on BitGraph graphs (namespace `bitgraph`):

```cpp
#include "graph/graph.h"
#include "clique_utils.h"

using namespace bitgraph;

ugraph ug{ 6 };                            // triangle {0, 3, 4} plus edges 0-1 and 2-5
ug.add_edge(0, 1);
ug.add_edge(0, 3);
ug.add_edge(0, 4);
ug.add_edge(2, 5);
ug.add_edge(3, 4);

ugraph::vertex_bitset_t bbsg{ ug.size(), true };    // all vertices

std::vector<int> clq;
clique_utils::find_clique_max_deg(ug, clq, bbsg);     // clq = {0, 3, 4}
int lb = clique_utils::find_clique_lb(ug);            // 2
int ub = clique_utils::ISEQ(ug, bbsg);                // 3 colors
bool ok = clique_utils::is_clique(ug, clq);           // true
```

See [`src/main_clq.cpp`](src/main_clq.cpp) and [`src/tests/test_clique_utils.cpp`](src/tests/test_clique_utils.cpp) for complete examples.

## API

| Function | Description |
|---|---|
| `clique_utils::extend_clique(g, clq, lv, begin, end)` | Adds to `clq` the vertices of `lv[begin, end)` that are adjacent to every vertex already in the clique. |
| `clique_utils::find_clique<GraphT, Reverse>(g, clq, bbsg)` | Greedy clique built from the vertices in the bitset `bbsg`, taken first to last (or last to first). |
| `clique_utils::find_clique_max_deg<GraphT, Reverse>(g, clq, bbsg)` | Greedy clique that each time picks the candidate with the most neighbors among the remaining candidates. |
| `clique_utils::find_clique_from_pool(g, clq, bbsg)` | Builds a greedy clique from every seed vertex and keeps the largest one. |
| `clique_utils::find_clique_lb<GraphT, Reverse>(g [, bbsg])` | Size of the greedy clique: a lower bound for the maximum clique of `g` (or of `g[bbsg]`). |
| `clique_utils::ISEQ(g, bbsg [, ub])` | Greedy sequential independent set coloring of `bbsg`: an upper bound for the maximum clique. Optionally returns each vertex's color (C array or `std::vector`). |
| `clique_utils::is_clique(g, set)` | Whether the set (bitset, `std::vector` or C array plus size) is a clique. The empty set is not a clique. |
| `clique_utils::is_clique(g, vertices, v)` | Whether `v` is adjacent to every vertex in `vertices` (true if empty). |
| `clique_utils::iset_prefix_size(g, vertices, begin, end)` | Number of consecutive vertices of `vertices[begin, end)` that form an independent set. |
| `clique_utils::iset_prefix_size(g, v, vertices, begin, end)` | Number of consecutive vertices of `vertices[begin, end)` that extend the independent set `{v}`. |
| `clique_utils::is_iset(g, set)` | Whether the set (bitset or `std::vector`) is an independent set. The empty set is not an independent set. |
| `clique_utils::can_extend_iset(g, vertices, v)` | Whether `v` is non-adjacent to every vertex in `vertices`. |

## Build

BitGraph and GoogleTest are fetched automatically by CMake (`FetchContent`), so you only need a C++14 compiler and CMake 3.14 or later.

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release        # run the unit tests
```

The executable `clq_utils` is written to `bin/`. Run it without arguments for a demo graph, or pass a DIMACS graph file:

```bash
bin/Release/clq_utils path/to/graph.clq
```

## Known issues

- None known.

## Project layout

```
src/clique_utils.h         clique and independent set functions (header-only library)
src/main_clq.cpp       demo executable
src/tests/             GoogleTest unit tests
CMakeLists.txt         fetches BitGraph and GoogleTest
```

## Author

[Pablo San Segundo](https://github.com/psanse)
