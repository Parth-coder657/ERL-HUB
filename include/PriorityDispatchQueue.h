#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ReliefOps {
namespace Logistics {

template <typename T>
class PriorityDispatchQueue {
public:
    struct Entry {
        int priority;
        bool urgentMedical;
        unsigned long sequence;
        T value;
    };

    void enqueue(const T& value, int priority, bool urgentMedical) {
        if (priority < 1 || priority > 5) {
            throw std::invalid_argument("Priority must be between 1 and 5.");
        }
        if (identities_.find(value.identity()) != identities_.end()) {
            throw std::invalid_argument("A request with that ID is already queued.");
        }
        const auto inserted = identities_.emplace(value.identity(), true);
        try {
            entries_.push_back(Entry{priority, urgentMedical, nextSequence_, value});
        } catch (...) {
            identities_.erase(inserted.first);
            throw;
        }
        ++nextSequence_;
    }

    bool empty() const noexcept { return entries_.empty(); }
    std::size_t size() const noexcept { return entries_.size(); }

    const Entry& next() const {
        if (entries_.empty()) {
            throw std::out_of_range("There are no queued mission requests.");
        }
        return *std::min_element(entries_.begin(), entries_.end(), HigherPriority{});
    }

    Entry pop() {
        if (entries_.empty()) {
            throw std::out_of_range("There are no queued mission requests.");
        }
        const auto best = std::min_element(entries_.begin(), entries_.end(), HigherPriority{});
        Entry selected = *best;
        identities_.erase(selected.value.identity());
        entries_.erase(best);
        return selected;
    }

    const std::vector<Entry>& entries() const noexcept { return entries_; }

    std::vector<Entry> orderedEntries() const {
        std::vector<Entry> ordered = entries_;
        std::sort(ordered.begin(), ordered.end(), HigherPriority{});
        return ordered;
    }

    void swap(PriorityDispatchQueue& other) noexcept {
        entries_.swap(other.entries_);
        identities_.swap(other.identities_);
        std::swap(nextSequence_, other.nextSequence_);
    }

private:
    struct HigherPriority {
        bool operator()(const Entry& left, const Entry& right) const noexcept {
            if (left.urgentMedical != right.urgentMedical) return left.urgentMedical;
            if (left.priority != right.priority) return left.priority > right.priority;
            return left.sequence < right.sequence;
        }
    };

    std::vector<Entry> entries_;
    std::unordered_map<std::string, bool> identities_;
    unsigned long nextSequence_ = 0;
};

} // namespace Logistics
} // namespace ReliefOps
