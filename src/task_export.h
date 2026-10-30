#pragma once

#include <string>
#include <vector>

#include "task_store.h"

std::string tasksToCsv(const std::vector<Task>& tasks);
bool tasksFromCsv(const std::string& csv, std::vector<Task>& tasks, std::string& error);
