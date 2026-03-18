/******************************************************************************
 *                                                                            *
 * Copyright (C) 2026 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_INTERVAL_H
#define MWR_UTILS_INTERVAL_H

#include "mwr/core/compiler.h"
#include "mwr/core/types.h"
#include "mwr/core/report.h"

namespace mwr {

template <typename T>
class interval_tree
{
private:
    static constexpr bool overlaps(u64 s0, u64 e0, u64 s1, u64 e1) {
        return s0 <= e1 && s1 <= e0;
    }

    struct ivtnode {
        u64 lo;
        u64 hi;
        u64 min;
        u64 max;
        size_t height;
        struct ivtnode* parent;
        struct ivtnode* left;
        struct ivtnode* right;

        T data;

        ivtnode(u64 start, u64 end, const T& item):
            lo(start),
            hi(end),
            min(start),
            max(end),
            height(1),
            parent(),
            left(),
            right(),
            data(item) {}

        ivtnode(u64 start, u64 end, T&& item):
            lo(start),
            hi(end),
            min(start),
            max(end),
            height(1),
            parent(),
            left(),
            right(),
            data(std::move(item)) {}
    };

    static size_t ivt_min(ivtnode* node) {
        u64 min = node->lo;
        if (node->left && node->left->min < min)
            min = node->left->min;
        if (node->right && node->right->min < min)
            min = node->right->min;
        return min;
    }

    static size_t ivt_max(ivtnode* node) {
        u64 max = node->hi;
        if (node->left && node->left->max > max)
            max = node->left->max;
        if (node->right && node->right->max > max)
            max = node->right->max;
        return max;
    }

    static size_t ivt_height(ivtnode* node) {
        if (node == nullptr)
            return 0;

        size_t lh = node->left ? node->left->height : 0;
        size_t rh = node->right ? node->right->height : 0;
        return 1 + max(lh, rh);
    }

    static ssize_t ivt_balance(ivtnode* node) {
        if (node == nullptr)
            return 0;

        size_t lh = node->left ? node->left->height : 0;
        size_t rh = node->right ? node->right->height : 0;
        return lh - rh;
    }

    static bool ivt_is_left(ivtnode* node) {
        return node == node->parent->left;
    }

    static bool ivt_is_right(ivtnode* node) {
        return node == node->parent->right;
    }

    static ivtnode* ivt_first(ivtnode* node) {
        while (node && node->left)
            node = node->left;
        return node;
    }

    static ivtnode* ivt_last(ivtnode* node) {
        while (node && node->right)
            node = node->right;
        return node;
    }

    static ivtnode* ivt_next(ivtnode* node) {
        if (node == nullptr)
            return nullptr;

        if (node->right)
            return ivt_first(node->right);

        while (node->parent && ivt_is_right(node))
            node = node->parent;
        return node->parent;
    }

    static ivtnode* ivt_prev(ivtnode* node) {
        if (node == nullptr)
            return nullptr;

        if (node->left)
            return ivt_last(node->left);

        while (node->parent && ivt_is_left(node))
            node = node->parent;
        return node->parent;
    }

    static ivtnode*& ivt_ref(ivtnode*& root, ivtnode* node) {
        if (node->parent == nullptr)
            return root;
        if (node == node->parent->left)
            return node->parent->left;
        return node->parent->right;
    }

    static void ivt_refresh(ivtnode* node) {
        if (node != nullptr) {
            node->height = ivt_height(node);
            node->min = ivt_min(node);
            node->max = ivt_max(node);
        }
    }

    static void ivt_transplant(ivtnode*& root, ivtnode* node,
                               ivtnode* parent) {
        root = node;

        if (node != nullptr)
            node->parent = parent;
    }

    static void ivt_rotate_left(ivtnode*& root) {
        ivtnode* oldroot = root;
        ivtnode* newroot = oldroot->right;
        ivtnode* subtree = newroot->left;
        ivt_transplant(root, newroot, root->parent);
        ivt_transplant(newroot->left, oldroot, newroot);
        ivt_transplant(oldroot->right, subtree, oldroot);
        ivt_refresh(oldroot);
        ivt_refresh(newroot);
    }

    static void ivt_rotate_right(ivtnode*& root) {
        ivtnode* oldroot = root;
        ivtnode* newroot = oldroot->left;
        ivtnode* subtree = newroot->right;
        ivt_transplant(root, newroot, root->parent);
        ivt_transplant(newroot->right, oldroot, newroot);
        ivt_transplant(oldroot->left, subtree, oldroot);
        ivt_refresh(oldroot);
        ivt_refresh(newroot);
    }

    static void ivt_rotate_left_right(ivtnode*& node) {
        ivt_rotate_left(node->left);
        ivt_rotate_right(node);
    }

    static void ivt_rotate_right_left(ivtnode*& node) {
        ivt_rotate_right(node->right);
        ivt_rotate_left(node);
    }

    static void ivt_rebalance(ivtnode*& node) {
        if (node == nullptr)
            return;

        ivt_refresh(node);

        ssize_t balance = ivt_balance(node);

        if (balance > 1) {
            if (ivt_balance(node->left) < 0)
                ivt_rotate_left_right(node);
            else
                ivt_rotate_right(node);
        }

        if (balance < -1) {
            if (ivt_balance(node->right) > 0)
                ivt_rotate_right_left(node);
            else
                ivt_rotate_left(node);
        }
    }

    static void ivt_insert(ivtnode*& root, ivtnode* node, ivtnode* parent) {
        if (node == nullptr)
            return;

        if (root == nullptr) {
            node->parent = parent;
            root = node;
            return;
        }

        if (node->lo <= root->lo)
            ivt_insert(root->left, node, root);
        else
            ivt_insert(root->right, node, root);

        ivt_rebalance(root);
    }

    static void ivt_remove(ivtnode*& root, ivtnode* node) {
        if (node == nullptr)
            return;

        ivtnode* rebalance = nullptr;

        if (node->left == nullptr) {
            rebalance = node->parent;
            ivt_transplant(ivt_ref(root, node), node->right, node->parent);
        } else if (node->right == nullptr) {
            rebalance = node->parent;
            ivt_transplant(ivt_ref(root, node), node->left, node->parent);
        } else {
            ivtnode* prev = ivt_prev(node);
            rebalance = prev;
            if (prev != node->left) {
                rebalance = prev->parent;
                ivt_transplant(ivt_ref(root, prev), prev->left, prev->parent);
                prev->left = node->left;
                prev->left->parent = prev;
            }

            ivt_transplant(ivt_ref(root, node), prev, node->parent);
            prev->right = node->right;
            if (prev->right)
                prev->right->parent = prev;
        }

        while (rebalance) {
            ivtnode* parent = rebalance->parent;
            ivt_rebalance(ivt_ref(root, rebalance));
            rebalance = parent;
        }
    }

    static void ivt_clear(ivtnode*& root) {
        if (root == nullptr)
            return;

        ivt_clear(root->left);
        ivt_clear(root->right);
        delete root;
        root = nullptr;
    }

    static bool ivt_overlaps(ivtnode* node, u64 start, u64 end) {
        if (node == nullptr)
            return false;

        if (!overlaps(node->min, node->max, start, end))
            return false;

        if (overlaps(node->lo, node->hi, start, end))
            return true;

        return ivt_overlaps(node->left, start, end) ||
               ivt_overlaps(node->right, start, end);
    }

    template <typename FN>
    static void ivt_for_each(ivtnode* node, u64 start, u64 end, FN fn) {
        if (node == nullptr)
            return;

        if (!overlaps(node->min, node->max, start, end))
            return;

        if (overlaps(node->lo, node->hi, start, end))
            fn(node->lo, node->hi, node);

        ivt_for_each(node->left, start, end, fn);
        ivt_for_each(node->right, start, end, fn);
    }

    static void ivt_validate(ivtnode* node, ivtnode* parent, size_t& count) {
        if (node == nullptr)
            return;

        ivt_validate(node->left, node, count);
        ivt_validate(node->right, node, count);

        if (node->parent != parent)
            MWR_ERROR("node parent pointer corrupted");
        if (node->min != ivt_min(node))
            MWR_ERROR("node minimum corrupted");
        if (node->max != ivt_max(node))
            MWR_ERROR("node maximum corrupted");
        ssize_t balance = ivt_balance(node);
        if (balance < -1 || balance > 1)
            MWR_ERROR("node is imbalanced");

        count++;
    }

    ivtnode* m_root;
    size_t m_size;

public:
    class const_iterator
    {
        friend class interval_tree;

    private:
        ivtnode* m_node;

    public:
        const_iterator(ivtnode* node): m_node(node) {}
        const_iterator& operator++() {
            m_node = ivt_next(m_node);
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const const_iterator& other) const {
            return m_node == other.m_node;
        }

        bool operator!=(const const_iterator& other) const {
            return m_node != other.m_node;
        }

        const T& operator*() const { return data(); }
        const T& data() const { return m_node->data; }
        u64 start() const { return m_node->lo; }
        u64 end() const { return m_node->hi; }
    };

    class iterator
    {
        friend class interval_tree;

    private:
        ivtnode* m_node;

    public:
        iterator(ivtnode* node): m_node(node) {}

        iterator& operator++() {
            m_node = ivt_next(m_node);
            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const iterator& other) const {
            return m_node == other.m_node;
        }

        bool operator!=(const iterator& other) const {
            return m_node != other.m_node;
        }

        T& operator*() const { return data(); }
        T& data() const { return m_node->data; }
        u64 start() const { return m_node->lo; }
        u64 end() const { return m_node->hi; }
    };

    iterator begin() { return iterator(ivt_first(m_root)); }
    iterator end() { return iterator(nullptr); }
    const_iterator begin() const { return const_iterator(ivt_first(m_root)); }
    const_iterator end() const { return const_iterator(nullptr); }

    interval_tree(): m_root(), m_size() {}
    ~interval_tree() { clear(); }

    interval_tree(interval_tree<T>&& other) noexcept:
        m_root(other.m_root), m_size(other.m_size) {
        other.m_root = nullptr;
        other.m_size = 0;
    }

    interval_tree& operator=(interval_tree<T>&& other) noexcept {
        if (this != &other) {
            clear();
            m_root = other.m_root;
            m_size = other.m_size;
            other.m_root = nullptr;
            other.m_size = 0;
        }
        return *this;
    }

    interval_tree(const interval_tree&) = delete;
    interval_tree& operator=(const interval_tree&) = delete;

    constexpr size_t size() const { return m_size; }
    constexpr bool empty() const { return m_size == 0; }

    void clear() {
        ivt_clear(m_root);
        m_size = 0;
    }

    iterator insert(u64 start, u64 end, const T& data) {
        MWR_ERROR_ON(end < start, "invalid interval %llu..%llu", start, end);
        ivtnode* node = new ivtnode(start, end, data);
        ivt_insert(m_root, node, nullptr);
        m_size++;
        return node;
    }

    iterator insert(u64 start, u64 end, T&& data) {
        MWR_ERROR_ON(end < start, "invalid interval %llu..%llu", start, end);
        ivtnode* node = new ivtnode(start, end, std::move(data));
        ivt_insert(m_root, node, nullptr);
        m_size++;
        return node;
    }

    bool remove(iterator it) {
        ivtnode* node = it.m_node;
        if (!node)
            return false;

        ivt_remove(m_root, node);
        delete it.m_node;
        m_size--;
        return true;
    }

    bool remove(const_iterator it) {
        ivtnode* node = it.m_node;
        if (!node)
            return false;

        ivt_remove(m_root, node);
        delete it.m_node;
        m_size--;
        return true;
    }

    bool overlaps(u64 start, u64 end) const {
        MWR_ERROR_ON(end < start, "invalid interval %llu..%llu", start, end);
        return ivt_overlaps(m_root, start, end);
    }

    void for_each(u64 start, u64 end,
                  function<void(u64, u64, const const_iterator&)> fn) const {
        MWR_ERROR_ON(end < start, "invalid interval %llu..%llu", start, end);
        ivt_for_each(m_root, start, end, fn);
    }

    void for_each(u64 start, u64 end,
                  function<void(u64, u64, const iterator&)> fn) {
        MWR_ERROR_ON(end < start, "invalid interval %llu..%llu", start, end);
        ivt_for_each(m_root, start, end, fn);
    }

    vector<const_iterator> find_overlaps(u64 start, u64 end) const {
        MWR_ERROR_ON(end < start, "invalid interval %llu..%llu", start, end);
        vector<const_iterator> result;
        for_each(start, end, [&result](u64, u64, const const_iterator& it) {
            result.push_back(it);
        });
        return result;
    }

    vector<iterator> find_overlaps(u64 start, u64 end) {
        MWR_ERROR_ON(end < start, "invalid interval %llu..%llu", start, end);
        vector<iterator> result;
        for_each(start, end, [&result](u64, u64, const iterator& it) {
            result.push_back(it);
        });
        return result;
    }

    void validate() const {
        size_t count = 0;
        ivt_validate(m_root, nullptr, count);
        MWR_ERROR_ON(count != m_size, "tree size corrupted");
    }
};

} // namespace mwr

#endif
