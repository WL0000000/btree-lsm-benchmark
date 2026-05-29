#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>

#include "btree.h"
#include "lsm_tree.h"

using Clock = std::chrono::high_resolution_clock;

double elapsed_ms(Clock::time_point start) {
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

// ---- Workload generators ----

std::vector<int> randomKeys(int n, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(1, n * 10);
    std::vector<int> keys(n);
    for (auto& k : keys) k = dist(rng);
    return keys;
}

std::vector<int> sequentialKeys(int n) {
    std::vector<int> keys(n);
    for (int i = 0; i < n; ++i) keys[i] = i + 1;
    return keys;
}

// ---- Benchmark templates ----

template<typename DS>
double benchInsert(DS& ds, const std::vector<int>& keys) {
    auto t = Clock::now();
    for (int k : keys) ds.insert(k, k * 2);
    return elapsed_ms(t);
}

template<typename DS>
double benchLookup(DS& ds, const std::vector<int>& keys) {
    volatile int sink = 0;
    auto t = Clock::now();
    for (int k : keys) {
        auto v = ds.search(k);
        if (v) sink += *v;
    }
    return elapsed_ms(t);
}

template<typename DS>
double benchRange(DS& ds, int n, int rangeWidth) {
    std::mt19937 rng(99);
    std::uniform_int_distribution<int> dist(1, n * 10 - rangeWidth);
    int numQueries = 1000;
    volatile size_t sink = 0;
    auto t = Clock::now();
    for (int i = 0; i < numQueries; ++i) {
        int lo = dist(rng);
        auto res = ds.range(lo, lo + rangeWidth);
        sink += res.size();
    }
    return elapsed_ms(t);
}

int main() {
    std::vector<int> sizes = {1000, 5000, 10000, 50000, 100000, 500000};

    // CSV output files
    std::ofstream ins_csv("../figures/data_insert.csv");
    std::ofstream lkp_csv("../figures/data_lookup.csv");
    std::ofstream rng_csv("../figures/data_range.csv");

    ins_csv << "n,btree_ms,lsm_ms\n";
    lkp_csv << "n,btree_ms,lsm_ms\n";
    rng_csv << "n,btree_ms,lsm_ms\n";

    for (int n : sizes) {
        std::cerr << "n = " << n << "...\n";
        auto keys   = randomKeys(n);
        auto lkeys  = randomKeys(n, 123); // lookup keys (some hits, some misses)

        // --- B-Tree ---
        BTree<int,int> bt;
        double bt_ins = benchInsert(bt, keys);
        double bt_lkp = benchLookup(bt, lkeys);
        double bt_rng = benchRange(bt, n, n / 10);

        // --- LSM-Tree ---
        LSMTree<int,int> lsm;
        double lsm_ins = benchInsert(lsm, keys);
        lsm.flush(); // flush remaining memtable before reads
        double lsm_lkp = benchLookup(lsm, lkeys);
        double lsm_rng = benchRange(lsm, n, n / 10);

        ins_csv << n << "," << std::fixed << std::setprecision(3)
                << bt_ins << "," << lsm_ins << "\n";
        lkp_csv << n << "," << bt_lkp << "," << lsm_lkp << "\n";
        rng_csv << n << "," << bt_rng << "," << lsm_rng << "\n";

        std::cout << "n=" << std::setw(7) << n
                  << "  BT_ins=" << std::setw(8) << bt_ins
                  << "ms  LSM_ins=" << std::setw(8) << lsm_ins
                  << "ms  BT_lkp=" << std::setw(8) << bt_lkp
                  << "ms  LSM_lkp=" << std::setw(8) << lsm_lkp
                  << "ms  BT_rng=" << std::setw(8) << bt_rng
                  << "ms  LSM_rng=" << std::setw(8) << lsm_rng << "ms\n";
    }

    std::cout << "\nCSV files written to ../figures/\n";
    return 0;
}
