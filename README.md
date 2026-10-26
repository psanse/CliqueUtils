<p align="center">
  <img alt="C++14" src="https://img.shields.io/badge/C%2B%2B-14-00599C?logo=c%2B%2B&logoColor=white">
  <img alt="CMake" src="https://img.shields.io/badge/build-CMake%20%E2%89%A5%203.14-064F8C?logo=cmake&logoColor=white">
  <img alt="BitGraph" src="https://img.shields.io/badge/BitGraph-master-informational">
</p>

# CliqueUtils: clique and independent set utilities for undirected graphs

Header-only C++ functions for cliques and independent sets: greedy heuristics, membership checks, a lower bound for the maximum clique and an upper bound from greedy sequential coloring. They are built on the bitset-based [BitGraph](https://github.com/psanse/BitGraph) library and are templated on the graph type.

> **Status:** work in progress. The unit tests pass with the BitGraph `master` branch (fetched by CMake); see [Known issues](#known-issues).

## Usage

Include [`src/clique_utils.h`](src/clique_utils.h). The functions live in the namespace `clique_utils` and work on BitGraph graphs (namespace `bitgraph`):

```cpp
#include "graph/graph.h"
#include "clique_utils.h"

bitgraph::ugraph ug{ 6 };                          // triangle {0, 3, 4} plus edges 0-1 and 2-5
ug.add_edge(0, 1);
ug.add_edge(0, 3);
ug.add_edge(0, 4);
ug.add_edge(2, 5);
ug.add_edge(3, 4);

bitgraph::ugraph::vertex_bitset_t bbsg{ ug.size(), true };    // all vertices

std::vector<int> clq;
clique_utils::find_clique_max_deg(ug, clq, bbsg);     // clq = {0, 3, 4}
int lb = clique_utils::find_clique_lb(ug);            // 2
int ub = clique_utils::greedy_coloring_iseq(ug, bbsg);   // 3 colors
bool ok = clique_utils::is_clique(ug, clq);           // true
```

See [`src/main_clique_utils.cpp`](src/main_clique_utils.cpp) and [`src/tests/test_clique_utils.cpp`](src/tests/test_clique_utils.cpp) for complete examples.

## API

| Function | Description |
|---|---|
| `clique_utils::extend_clique(graph, clq, vertices, begin, end)` | Adds to `clq` the vertices of the array `vertices[begin, end)` that are adjacent to every vertex already in the clique. Returns the number of vertices added. |
| `clique_utils::find_clique<GraphT, Reverse>(graph, clq, vertices)` | Greedy clique built from the vertices in the bitset `vertices`, taken first to last (or last to first). |
| `clique_utils::find_clique_max_deg<GraphT, Reverse>(graph, clq, vertices)` | Greedy clique that each time picks the candidate with the most neighbors among the remaining candidates. |
| `clique_utils::find_clique_from_pool(graph, clq, vertices)` | Builds a greedy clique from every seed vertex in `vertices` and keeps the largest one. |
| `clique_utils::find_clique_lb<GraphT, Reverse>(graph [, vertices])` | Size of the greedy clique: a lower bound for the maximum clique of `graph` (or of `graph[vertices]`). |
| `clique_utils::greedy_coloring_iseq(graph, vertices [, colors])` | Greedy sequential independent set coloring of `vertices`: an upper bound for the maximum clique. Optionally returns each vertex's color in `colors` (C array or `std::vector`). |
| `clique_utils::is_clique(graph, vertices [, size])` | Whether `vertices` (bitset, `std::vector`, or C array with `size` entries) is a clique. The empty set is not a clique. |
| `clique_utils::can_extend_clique(graph, clq, v)` | Whether `v` is adjacent to every vertex in the clique `clq` (true if empty). |
| `clique_utils::iset_prefix_size(graph, vertices, begin, end)` | Number of consecutive vertices of `vertices[begin, end)` that form an independent set. |
| `clique_utils::iset_prefix_size(graph, v, vertices, begin, end)` | Number of consecutive vertices of `vertices[begin, end)` that extend the independent set `{v}`. |
| `clique_utils::is_iset(graph, vertices)` | Whether `vertices` (bitset or `std::vector`) is an independent set. The empty set is not an independent set. |
| `clique_utils::can_extend_iset(graph, vertices, v)` | Whether `v` is non-adjacent to every vertex in `vertices`. |

## Build

BitGraph and GoogleTest are fetched automatically by CMake (`FetchContent`), so you only need a C++14 compiler and CMake 3.14 or later.

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release        # run the unit tests
```

The executable `clique_utils` is written to `bin/`. Run it without arguments for a demo graph, or pass a DIMACS graph file:

```bash
bin/Release/clique_utils path/to/graph.clq
```

## Known issues

- None known.

## Project layout

```
src/clique_utils.h        clique and independent set functions (header-only library)
src/main_clique_utils.cpp  demo executable
src/tests/             GoogleTest unit tests
CMakeLists.txt         fetches BitGraph and GoogleTest
```

## Author

[Pablo San Segundo](https://github.com/psanse)
