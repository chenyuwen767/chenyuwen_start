#pragma once

#include <cstddef>
#include <deque>
#include <vector>

#include "task_store.h"

// Keeps the last N task-list snapshots for "undo last change".
class TaskHistory {
public:
    explicit TaskHistory(std::size_t limit = 30);

    void record(const std::vector<Task>& state);
    bool canUndo() const;
    bool undo(std::vector<Task>& state);
    void clear();

private:
    std::deque<std::vector<Task>> states_;
    std::size_t limit_;
};
