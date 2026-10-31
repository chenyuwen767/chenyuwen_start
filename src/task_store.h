#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct Task {
    std::string title;
    std::string category;
    std::string difficulty;
    bool completed = false;
    std::vector<std::string> tags = {};
    std::string notes = {};
    std::string id = {};
    std::string platform = {};
    std::string problemId = {};
    std::string createdAt = {};
    std::string completedAt = {};
};

struct LoadResult {
    std::vector<Task> tasks;
    bool ok = true;
    std::string error;
    std::vector<std::string> warnings;
};

LoadResult loadTasksChecked(const std::filesystem::path& fileName);
std::vector<Task> loadTasks(const std::filesystem::path& fileName);
bool saveTasksChecked(const std::vector<Task>& tasks,
                      const std::filesystem::path& fileName,
                      std::string& error);
bool saveTasks(const std::vector<Task>& tasks, const std::filesystem::path& fileName);
