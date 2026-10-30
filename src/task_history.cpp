#include "task_history.h"

#include <utility>

TaskHistory::TaskHistory(std::size_t limit) : limit_(limit) {}

void TaskHistory::record(const std::vector<Task>& state) {
    if (limit_ == 0) {
        return;
    }
    states_.push_back(state);
    while (states_.size() > limit_) {
        states_.pop_front();
    }
}

bool TaskHistory::canUndo() const {
    return !states_.empty();
}

bool TaskHistory::undo(std::vector<Task>& state) {
    if (states_.empty()) {
        return false;
    }
    state = std::move(states_.back());
    states_.pop_back();
    return true;
}

void TaskHistory::clear() {
    states_.clear();
}
