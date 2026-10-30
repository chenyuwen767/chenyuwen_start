#pragma once

#include <string>
#include <vector>

#include "task_store.h"

std::string tasksToCsv(const std::vector<Task>& tasks);
