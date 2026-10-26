<p align="center">
  <img alt="C++14" src="https://img.shields.io/badge/C%2B%2B-14-00599C?logo=c%2B%2B&logoColor=white">
  <img alt="CMake" src="https://img.shields.io/badge/build-CMake%20%E2%89%A5%203.14-064F8C?logo=cmake&logoColor=white">
  <img alt="BitGraph" src="https://img.shields.io/badge/BitGraph-v2.0.0-informational">
</p>

# CliqueUtils: clique and independent set utilities for undirected graphs

Header-only C++ functions for cliques and independent sets: greedy heuristics, membership checks, a lower bound for the maximum clique and an upper bound from greedy sequential coloring. They are built on the bitset-based [BitGraph](https://github.com/psanse/BitGraph) library and are templated on the graph type.

> **Status:** work in progress. The unit tests pass with BitGraph v2.0.0; see [Known issues](#known-issues).

## Usage

Include [`src/clq_func.h`](src/clq_func.h). The functions live in the namespace `clique_utils` and work on BitGraph graphs (namespace `bitgraph`):

```cpp
#include "graph/graph.h"
#include "clq_func.h"

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

See [`src/main_clq.cpp`](src/main_clq.cpp) and [`src/tests/test_clq_func.cpp`](src/tests/test_clq_func.cpp) for complete examples.

## API

| Function | Description |
|---|---|
| `clique_utils::find_clique(g, clq, lv, begin, end)` | Adds to `clq` the vertices of `lv[begin, end)` that are adjacent to every vertex already in the clique. |
| `clique_utils::find_clique<GraphT, Reverse>(g, clq, bbsg)` | Greedy clique built from the vertices in the bitset `bbsg`, taken first to last (or last to first). |
| `clique_utils::find_clique_max_deg<GraphT, Reverse>(g, clq, bbsg)` | Greedy clique that each time picks the candidate with the most neighbors among the remaining candidates. |
| `clique_utils::find_clique_from_pool(g, clq, bbsg)` | Builds a greedy clique from every seed vertex and keeps the largest one. |
| `clique_utils::find_clique_lb<GraphT, Reverse>(g [, bbsg])` | Size of the greedy clique: a lower bound for the maximum clique of `g` (or of `g[bbsg]`). |
| `clique_utils::ISEQ(g, bbsg [, ub])` | Greedy sequential independent set coloring of `bbsg`: an upper bound for the maximum clique. Optionally returns each vertex's color (C array or `std::vector`). |
| `clique_utils::is_clique(g, set [, v])` | Whether the set (bitset, `std::vector` or C array), plus vertex `v` if given, is a clique. The empty set is not a clique. |
| `clique_utils::find_iset(g, lv, begin, end)` | Size of the independent set formed by the first consecutive vertices of `lv[begin, end)`. |
| `clique_utils::find_iset(g, v, lv, begin, end)` | Number of consecutive vertices of `lv[begin, end)` that extend the independent set `{v}`. |
| `clique_utils::is_iset(g, set [, v])` | Whether the set (bitset or `std::vector`), plus vertex `v` if given, is an independent set. The empty set is not an independent set. |

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

- `clique_utils::find_clique_from_pool` narrows the candidates with the neighborhood of the seed vertex instead of the vertex just added, so it can return sets that are not cliques. The unit test `Clique.find_clique_in_pool` currently expects that wrong result (`{0, 1, 3, 4}`).
- `clique_utils::is_clique` for bitsets only checks that each vertex has at least one neighbor in the set, so it returns true for some sets that are not cliques (e.g. `{0, 1, 3}` in the demo graph).
- `clique_utils::find_iset(g, v, lv, begin, end)` reads out of bounds when `begin == 0`, because the inner loop counter is unsigned.

## Project layout

```
src/clq_func.h         clique and independent set functions (header-only library)
src/main_clq.cpp       demo executable
src/tests/             GoogleTest unit tests
CMakeLists.txt         fetches BitGraph and GoogleTest
```

## Author

[Pablo San Segundo](https://github.com/psanse)
