#include "task_store.h"

#include <fstream>
#include <sstream>

std::vector<Task> loadTasks(const std::string& fileName) {
    std::vector<Task> tasks;
    std::ifstream input(fileName);
    std::string line;

    while (std::getline(input, line)) {
        std::istringstream fields(line);
        Task task;
        std::string completedText;

        if (std::getline(fields, task.title, '\t') &&
            std::getline(fields, task.category, '\t') &&
            std::getline(fields, task.difficulty, '\t') &&
            std::getline(fields, completedText)) {
            task.completed = completedText == "1";
            tasks.push_back(task);
        }
    }

    return tasks;
}

bool saveTasks(const std::vector<Task>& tasks, const std::string& fileName) {
    std::ofstream output(fileName);
    if (!output) {
        return false;
    }

    for (const Task& task : tasks) {
        output << task.title << '\t' << task.category << '\t'
               << task.difficulty << '\t' << (task.completed ? "1" : "0") << '\n';
    }
    return true;
}
