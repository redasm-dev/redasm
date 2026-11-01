#pragma once

#include <QList>

enum class ChangeState { UNCHANGED, ADDED, MODIFIED, DELETED };

template<typename T, typename Equal = std::equal_to<T>>
class ChangeTracker {
    struct Item {
        bool added;
        T original;
        T current;
    };

public:
    struct Change {
        ChangeState state;
        T original; // for MODIFIED / DELETED
        T current;  // for ADDED / MODIFIED
    };

    explicit ChangeTracker(Equal equal = Equal{}): m_equal{std::move(equal)} {}

    void reset(const QList<T>& items) {
        m_items.clear();
        m_deleted.clear();
        m_items.reserve(items.size());

        for(const T& v : items)
            m_items.push_back({false, v, v});
    }

    [[nodiscard]] qsizetype size() const { return m_items.size(); }

    const T& at(qsizetype row) const { return m_items[row].current; }
    T& at(qsizetype row) { return m_items[row].current; } // edit in place
    const T& original(qsizetype row) const { return m_items[row].original; }

    [[nodiscard]] ChangeState state(qsizetype row) const {
        return this->state_of(m_items[row]);
    }

    void insert(qsizetype row, qsizetype count, const T& value = T{}) {
        m_items.insert(row, count, Item{true, value, value});
    }

    void remove(qsizetype row, qsizetype count) {
        for(qsizetype i = row; i < row + count; ++i) {
            if(!m_items[i].added)
                m_deleted.push_back(m_items[i]); // added-then-deleted: drop
        }

        m_items.remove(row, count);
    }

    // Deletions first, so "delete X, then add X again" applies cleanly
    QList<Change> changes() const {
        QList<Change> res;
        for(const Item& d : m_deleted)
            res.push_back({ChangeState::DELETED, d.original, d.current});

        for(const Item& it : m_items) {
            const ChangeState S = this->state_of(it);

            if(S != ChangeState::UNCHANGED)
                res.push_back({S, it.original, it.current});
        }

        return res;
    }

    [[nodiscard]] bool has_changes() const {
        if(!m_deleted.isEmpty()) return true;

        for(const Item& it : m_items) {
            if(this->state_of(it) != ChangeState::UNCHANGED) return true;
        }

        return false;
    }

    ChangeState state_of(const Item& it) const {
        if(it.added) return ChangeState::ADDED;
        return m_equal(it.original, it.current) ? ChangeState::UNCHANGED
                                                : ChangeState::MODIFIED;
    }

private:
    Equal m_equal;
    QList<Item> m_items;
    QList<Item> m_deleted;
};
