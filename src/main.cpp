#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "task_store.h"

namespace {

const char* kDataFile = "tasks.txt";

std::string ask(const std::string& prompt) {
    std::cout << prompt;
    std::string answer;
    std::getline(std::cin, answer);
    return answer;
}

void printTasks(const std::vector<Task>& tasks, bool onlyPending) {
    bool found = false;
    for (std::size_t index = 0; index < tasks.size(); ++index) {
        const Task& task = tasks[index];
        if (onlyPending && task.completed) {
            continue;
        }

        found = true;
        std::cout << index + 1 << ". [" << (task.completed ? "已完成" : "待完成") << "] "
                  << task.title << " | " << task.category << " | " << task.difficulty << '\n';
    }

    if (!found) {
        std::cout << (onlyPending ? "没有待完成的题目。\n" : "还没有记录题目。\n");
    }
}

void addTask(std::vector<Task>& tasks) {
    Task task;
    task.title = ask("题目名称：");
    task.category = ask("分类：");
    task.difficulty = ask("难度：");

    if (task.title.empty() || task.category.empty() || task.difficulty.empty()) {
        std::cout << "题目名称、分类和难度都不能为空，未新增。\n";
        return;
    }

    tasks.push_back(task);
    std::cout << "已新增：" << task.title << '\n';
}

void completeTask(std::vector<Task>& tasks) {
    if (tasks.empty()) {
        std::cout << "还没有题目可以标记。\n";
        return;
    }

    printTasks(tasks, false);
    std::cout << "输入要完成的题目编号：";
    std::size_t number;
    if (!(std::cin >> number)) {
        std::cout << "请输入数字编号。\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (number == 0 || number > tasks.size()) {
        std::cout << "编号不存在。\n";
        return;
    }

    Task& task = tasks[number - 1];
    task.completed = true;
    std::cout << "已完成：" << task.title << '\n';
}

void printMenu() {
    std::cout << "\n=== 算法练习任务本 ===\n"
              << "1. 新增题目\n"
              << "2. 查看全部题目\n"
              << "3. 查看待完成题目\n"
              << "4. 标记题目为完成\n"
              << "0. 保存并退出\n";
}

}

int main() {
    std::vector<Task> tasks = loadTasks(kDataFile);
    std::cout << "已加载 " << tasks.size() << " 道题目。\n";

    while (true) {
        printMenu();
        std::cout << "请选择：";
        int choice;
        if (!(std::cin >> choice)) {
            std::cout << "请输入菜单中的数字。\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                addTask(tasks);
                break;
            case 2:
                printTasks(tasks, false);
                break;
            case 3:
                printTasks(tasks, true);
                break;
            case 4:
                completeTask(tasks);
                break;
            case 0:
                if (saveTasks(tasks, kDataFile)) {
                    std::cout << "已保存到 " << kDataFile << "，再见！\n";
                    return 0;
                }
                std::cerr << "保存失败，请检查当前目录的写入权限。\n";
                return 1;
            default:
                std::cout << "没有这个选项，请重新输入。\n";
        }
    }
}
