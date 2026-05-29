#pragma once
#include <vector>
#include <algorithm>
#include <cassert>
#include <optional>

// B-Tree of order T (minimum degree):
//   - Every non-root node has at least T-1 keys
//   - Every node has at most 2T-1 keys
//   - All leaves are at the same depth
template <typename K, typename V, int T = 64>
class BTree {
public:
    struct Node {
        std::vector<K>     keys;
        std::vector<V>     vals;
        std::vector<Node*> children;
        bool               leaf;

        Node(bool isLeaf) : leaf(isLeaf) {
            keys.reserve(2 * T - 1);
            vals.reserve(2 * T - 1);
            if (!isLeaf) children.reserve(2 * T);
        }
        ~Node() {
            for (auto* c : children) delete c;
        }
    };

private:
    Node* root_ = nullptr;
    size_t size_ = 0;

    // ---- search ----
    std::optional<V> search(Node* x, const K& k) const {
        int i = (int)(std::lower_bound(x->keys.begin(), x->keys.end(), k) - x->keys.begin());
        if (i < (int)x->keys.size() && x->keys[i] == k)
            return x->vals[i];
        if (x->leaf) return std::nullopt;
        return search(x->children[i], k);
    }

    // ---- split child ----
    void splitChild(Node* x, int i) {
        Node* y = x->children[i];
        Node* z = new Node(y->leaf);

        // move median key up
        x->keys.insert(x->keys.begin() + i, y->keys[T - 1]);
        x->vals.insert(x->vals.begin() + i, y->vals[T - 1]);
        x->children.insert(x->children.begin() + i + 1, z);

        // give right half to z
        z->keys.assign(y->keys.begin() + T, y->keys.end());
        z->vals.assign(y->vals.begin() + T, y->vals.end());
        y->keys.resize(T - 1);
        y->vals.resize(T - 1);

        if (!y->leaf) {
            z->children.assign(y->children.begin() + T, y->children.end());
            y->children.resize(T);
        }
    }

    // ---- insert into non-full ----
    void insertNonFull(Node* x, const K& k, const V& v) {
        int i = (int)x->keys.size() - 1;
        if (x->leaf) {
            auto it = std::lower_bound(x->keys.begin(), x->keys.end(), k);
            int pos = (int)(it - x->keys.begin());
            if (pos < (int)x->keys.size() && x->keys[pos] == k) {
                x->vals[pos] = v; // update
                size_--;
                return;
            }
            x->keys.insert(x->keys.begin() + pos, k);
            x->vals.insert(x->vals.begin() + pos, v);
        } else {
            // find child to descend into
            auto it = std::lower_bound(x->keys.begin(), x->keys.end(), k);
            int ci = (int)(it - x->keys.begin());
            if (ci < (int)x->keys.size() && x->keys[ci] == k) {
                x->vals[ci] = v; // update
                size_--;
                return;
            }
            if ((int)x->children[ci]->keys.size() == 2 * T - 1) {
                splitChild(x, ci);
                if (k > x->keys[ci]) ci++;
            }
            insertNonFull(x->children[ci], k, v);
        }
    }

    // ---- range query helper ----
    void rangeHelper(Node* x, const K& lo, const K& hi, std::vector<std::pair<K,V>>& out) const {
        int i = (int)(std::lower_bound(x->keys.begin(), x->keys.end(), lo) - x->keys.begin());
        for (; i < (int)x->keys.size() && x->keys[i] <= hi; ++i) {
            if (!x->leaf) rangeHelper(x->children[i], lo, hi, out);
            if (x->keys[i] >= lo) out.push_back({x->keys[i], x->vals[i]});
        }
        if (!x->leaf) rangeHelper(x->children[i], lo, hi, out);
    }

public:
    BTree() {
        root_ = new Node(true);
    }
    ~BTree() { delete root_; }

    // Disable copy
    BTree(const BTree&) = delete;
    BTree& operator=(const BTree&) = delete;

    void insert(const K& k, const V& v) {
        size_++;
        if ((int)root_->keys.size() == 2 * T - 1) {
            Node* s = new Node(false);
            s->children.push_back(root_);
            root_ = s;
            splitChild(root_, 0);
        }
        insertNonFull(root_, k, v);
    }

    std::optional<V> search(const K& k) const {
        return search(root_, k);
    }

    std::vector<std::pair<K,V>> range(const K& lo, const K& hi) const {
        std::vector<std::pair<K,V>> result;
        rangeHelper(root_, lo, hi, result);
        std::sort(result.begin(), result.end());
        return result;
    }

    size_t size() const { return size_; }
};
