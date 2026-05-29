#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include <optional>
#include <string>
#include <functional>

// Simplified LSM-Tree (Log-Structured Merge-Tree)
//
// Architecture:
//   Level 0  : MemTable (std::map, in-memory, mutable)
//   Level 1+ : Sorted Runs (immutable sorted arrays, simulating disk levels)
//
// Write path : insert into MemTable -> when full, flush to L0 run -> compact
// Read path  : check MemTable -> check runs newest-first
//
// This is an instructional implementation. A production LSM (like RocksDB)
// adds bloom filters, block compression, WAL logging, and compaction policies.

template <typename K, typename V>
class LSMTree {
public:
    // A single sorted run — immutable once created
    struct Run {
        std::vector<std::pair<K,V>> data; // sorted by key

        std::optional<V> get(const K& k) const {
            auto it = std::lower_bound(data.begin(), data.end(),
                std::make_pair(k, V{}),
                [](const auto& a, const auto& b){ return a.first < b.first; });
            if (it != data.end() && it->first == k) return it->second;
            return std::nullopt;
        }

        std::vector<std::pair<K,V>> range(const K& lo, const K& hi) const {
            std::vector<std::pair<K,V>> out;
            auto it = std::lower_bound(data.begin(), data.end(),
                std::make_pair(lo, V{}),
                [](const auto& a, const auto& b){ return a.first < b.first; });
            for (; it != data.end() && it->first <= hi; ++it)
                out.push_back(*it);
            return out;
        }
    };

private:
    static constexpr int MEMTABLE_CAPACITY = 512; // flush threshold
    static constexpr int LEVEL_RATIO       = 10;  // size ratio between levels

    // Level 0: mutable MemTable (red-black tree)
    std::map<K,V> memtable_;

    // Levels 1+: list of runs per level (index 0 = L1, newest-first within level)
    std::vector<std::vector<Run>> levels_;

    size_t size_ = 0;

    // Merge k sorted runs into one (k-way merge)
    Run mergeRuns(std::vector<Run>& runs) {
        // Collect all entries, later runs (newer) win on duplicate keys
        std::map<K,V> merged;
        for (auto& run : runs)
            for (auto& [k, v] : run.data)
                merged[k] = v; // newer writes overwrite older

        Run result;
        result.data.assign(merged.begin(), merged.end());
        return result;
    }

    // Flush MemTable -> L1 run, then compact if needed
    void flushMemTable() {
        if (memtable_.empty()) return;

        Run r;
        r.data.assign(memtable_.begin(), memtable_.end());
        memtable_.clear();

        if (levels_.empty()) levels_.emplace_back();
        levels_[0].push_back(std::move(r));

        compact(0);
    }

    // Cascade compaction: if level l has >= LEVEL_RATIO runs, merge into l+1
    void compact(int l) {
        if (l >= (int)levels_.size()) return;
        if ((int)levels_[l].size() < LEVEL_RATIO) return;

        // Merge all runs on this level
        Run merged = mergeRuns(levels_[l]);
        levels_[l].clear();

        // Push to next level
        if (l + 1 >= (int)levels_.size()) levels_.emplace_back();
        levels_[l + 1].push_back(std::move(merged));

        compact(l + 1);
    }

public:
    LSMTree() {}

    void insert(const K& k, const V& v) {
        memtable_[k] = v;
        size_++;
        if ((int)memtable_.size() >= MEMTABLE_CAPACITY)
            flushMemTable();
    }

    std::optional<V> search(const K& k) const {
        // 1. MemTable
        auto it = memtable_.find(k);
        if (it != memtable_.end()) return it->second;

        // 2. Runs from newest to oldest
        for (int l = 0; l < (int)levels_.size(); ++l)
            for (int r = (int)levels_[l].size() - 1; r >= 0; --r) {
                auto res = levels_[l][r].get(k);
                if (res) return res;
            }
        return std::nullopt;
    }

    // Range query: must check all levels and merge results
    std::vector<std::pair<K,V>> range(const K& lo, const K& hi) const {
        std::map<K,V> result;

        // MemTable
        auto it = memtable_.lower_bound(lo);
        for (; it != memtable_.end() && it->first <= hi; ++it)
            result[it->first] = it->second;

        // Runs (older first so newer overwrites)
        for (int l = (int)levels_.size() - 1; l >= 0; --l)
            for (int r = 0; r < (int)levels_[l].size(); ++r) {
                auto partial = levels_[l][r].range(lo, hi);
                for (auto& [k, v] : partial) result[k] = v;
            }

        return std::vector<std::pair<K,V>>(result.begin(), result.end());
    }

    void flush() { flushMemTable(); }

    size_t size() const { return size_; }

    // Stats for analysis
    int numLevels() const { return (int)levels_.size(); }
    int numRuns() const {
        int total = 0;
        for (auto& l : levels_) total += (int)l.size();
        return total;
    }
};
