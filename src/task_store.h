#pragma once

#include <string>
#include <vector>

struct Task {
    std::string title;
    std::string category;
    std::string difficulty;
    bool completed = false;
};

std::vector<Task> loadTasks(const std::string& fileName);
bool saveTasks(const std::vector<Task>& tasks, const std::string& fileName);
