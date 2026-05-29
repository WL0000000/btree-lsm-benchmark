# btree-lsm-benchmark

A from-scratch C++17 implementation and empirical benchmark of two foundational storage engine data structures: **B-Trees** and **Log-Structured Merge-Trees (LSM-Trees)**. Built as a research project for CMPT 225 at Simon Fraser University.

---

## Overview

Every major database — PostgreSQL, LevelDB, RocksDB, Cassandra — makes a fundamental architectural choice: which data structure sits at the heart of its storage engine? This project implements both dominant approaches and measures the trade-offs directly.

The core tension is formalized by the **RUM conjecture**: no data structure can simultaneously minimize *read overhead*, *update overhead*, and *memory overhead*. B-Trees optimize for reads; LSM-Trees optimize for writes. This repo makes that trade-off visible and measurable.

---

## Key Results

Benchmarked at `n = 500,000` keys (1,000 queries per workload):

| Operation    | B-Tree   | LSM-Tree  | Speedup       |
|--------------|----------|-----------|---------------|
| Insert       | 111 ms   | 231 ms    | 2.1× (B-Tree) |
| Point Lookup | 113 ms   | 1,151 ms  | ~10× (B-Tree) |
| Range Query  | 77 ms    | 827 ms    | ~10.7× (B-Tree) |

> **Note:** These are in-memory benchmarks. On spinning disk hardware, LSM-Tree writes are typically **5–100× faster** than B-Tree writes due to sequential I/O — the advantage our implementation does not model. See the full report for discussion.

---

## Project Structure

```
btree-lsm-benchmark/
├── src/
│   ├── btree.h          # Header-only B-Tree (min degree T=64, generic K/V)
│   ├── lsm_tree.h       # Header-only LSM-Tree (leveled compaction, ratio=10)
│   └── benchmark.cpp    # Benchmark driver — insert, lookup, range workloads
├── figures/
│   ├── fig_insert.pdf
│   ├── fig_lookup.pdf
│   ├── fig_range.pdf
│   └── fig_summary.pdf
├── results/
│   └── benchmark_results.csv
├── report/
│   └── report.pdf       # Full research report with complexity analysis
└── README.md
```

---

## Build & Run

**Requirements:** `g++` with C++17 support (`g++ --version` ≥ 7)

```bash
# Clone the repo
git clone https://github.com/your-username/btree-lsm-benchmark.git
cd btree-lsm-benchmark/src

# Compile
g++ -O2 -std=c++17 benchmark.cpp -o benchmark

# Run
./benchmark
```

Output is printed to stdout and written to `results/benchmark_results.csv`.

---

## Implementation Details

### B-Tree (`btree.h`)
- Generic template: `BTree<K, V, int T = 64>`
- Top-down splitting on insert — no backtracking needed
- `std::lower_bound` binary search within nodes: O(log 2T) comparisons per level
- In-order traversal for range queries
- Height ≤ log₆₄(n/2) — at most **3 levels** for 1 million keys

### LSM-Tree (`lsm_tree.h`)
- `std::map` MemTable (red-black tree, O(log M) inserts)
- Sorted runs stored as `std::vector<pair<K,V>>` (simulating SSTs)
- Leveled compaction with configurable `LEVEL_RATIO` (default: 10, matching LevelDB)
- N-way merge on compaction; newest value wins on duplicate keys
- MemTable capacity: 512 entries (configurable)

### What's not implemented (by design)
- No disk persistence or buffer pool — both structures run in memory for a fair comparison
- No bloom filters — LSM-Tree read performance is therefore a **worst case** vs. production LevelDB/RocksDB
- No concurrency — single-threaded benchmark only

---

## When to Use Each

| Workload | Recommended | Why |
|---|---|---|
| Read-heavy / OLTP | **B-Tree** | O(log n) lookup, excellent range scans via leaf links |
| Write-heavy / append logs | **LSM-Tree** | Sequential disk writes, 5–100× throughput on HDD |
| Time-series / event logs | **LSM-Tree** | Natural fit for Cassandra, HBase, RocksDB-backed systems |
| Relational DB (PostgreSQL, SQLite) | **B-Tree** | Mixed read/write, ordered scans required |

---

## Background Reading

The report covers the full history and theory — from the original 1972 B-Tree paper through recent advances:

- Bayer & McCreight (1972) — original B-Tree paper
- O'Neil et al. (1996) — LSM-Tree proposal
- Comer (1979) — B-Tree survey and B⁺-Tree formalization
- Athanassoulis et al. (2016) — RUM conjecture
- Dayan et al. (2017) — Monkey (optimal bloom filter allocation)
- Dayan & Idreos (2018) — Dostoevsky (Fluid LSM-Tree)

Full citations in [`report/report.pdf`](report/report.pdf).

---

## License

MIT
