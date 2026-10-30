#include "task_store.h"

#include <fstream>
#include <sstream>
#include <system_error>
#include <utility>

namespace {

constexpr const char* kFileHeader = "# algorithm-practice-notebook v2";

std::string escapeField(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char character : value) {
        switch (character) {
        case '\\':
            escaped += "\\\\";
            break;
        case '\t':
            escaped += "\\t";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        default:
            escaped += character;
            break;
        }
    }
    return escaped;
}

std::string unescapeField(const std::string& value) {
    std::string unescaped;
    unescaped.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] != '\\' || index + 1 >= value.size()) {
            unescaped += value[index];
            continue;
        }

        switch (value[++index]) {
        case 't':
            unescaped += '\t';
            break;
        case 'n':
            unescaped += '\n';
            break;
        case 'r':
            unescaped += '\r';
            break;
        case '\\':
            unescaped += '\\';
            break;
        default:
            unescaped += '\\';
            unescaped += value[index];
            break;
        }
    }
    return unescaped;
}

} // namespace

LoadResult loadTasksChecked(const std::filesystem::path& fileName) {
    LoadResult result;
    std::error_code existsError;
    if (!std::filesystem::exists(fileName, existsError)) {
        if (existsError) {
            result.ok = false;
            result.error = "无法检查数据文件：" + existsError.message();
        }
        return result;
    }

    std::ifstream input(fileName, std::ios::binary);
    if (!input) {
        result.ok = false;
        result.error = "无法打开数据文件：" + fileName.string();
        return result;
    }

    std::string line;
    int lineNumber = 0;
    bool escapedFormat = false;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (lineNumber == 1 && line.size() >= 3 &&
            static_cast<unsigned char>(line[0]) == 0xEF &&
            static_cast<unsigned char>(line[1]) == 0xBB &&
            static_cast<unsigned char>(line[2]) == 0xBF) {
            line.erase(0, 3);
        }
        if (lineNumber == 1 && line == kFileHeader) {
            escapedFormat = true;
            continue;
        }
        if (line.empty()) {
            continue;
        }

        std::istringstream fields(line);
        Task task;
        std::string completedText;

        if (!std::getline(fields, task.title, '\t') ||
            !std::getline(fields, task.category, '\t') ||
            !std::getline(fields, task.difficulty, '\t') ||
            !std::getline(fields, completedText)) {
            result.warnings.push_back("第 " + std::to_string(lineNumber) + " 行字段不足，已跳过");
            continue;
        }

        if (escapedFormat) {
            task.title = unescapeField(task.title);
            task.category = unescapeField(task.category);
            task.difficulty = unescapeField(task.difficulty);
        }
        task.completed = completedText == "1";
        result.tasks.push_back(std::move(task));
    }

    if (input.bad()) {
        result.ok = false;
        result.error = "读取数据文件时发生错误：" + fileName.string();
    }
    return result;
}

std::vector<Task> loadTasks(const std::filesystem::path& fileName) {
    return loadTasksChecked(fileName).tasks;
}

bool saveTasksChecked(const std::vector<Task>& tasks,
                      const std::filesystem::path& fileName,
                      std::string& error) {
    std::error_code errorCode;
    std::filesystem::path temporaryFile = fileName;
    temporaryFile += ".tmp";
    std::filesystem::path backupFile = fileName;
    backupFile += ".bak";

    {
        std::ofstream output(temporaryFile, std::ios::binary | std::ios::trunc);
        if (!output) {
            error = "无法创建临时数据文件：" + temporaryFile.string();
            return false;
        }

        output << kFileHeader << '\n';
        for (const Task& task : tasks) {
            output << escapeField(task.title) << '\t'
                   << escapeField(task.category) << '\t'
                   << escapeField(task.difficulty) << '\t'
                   << (task.completed ? "1" : "0") << '\n';
        }

        output.flush();
        if (!output) {
            error = "写入临时数据文件失败：" + temporaryFile.string();
            output.close();
            std::filesystem::remove(temporaryFile, errorCode);
            return false;
        }
        output.close();
        if (!output) {
            error = "关闭临时数据文件失败：" + temporaryFile.string();
            std::filesystem::remove(temporaryFile, errorCode);
            return false;
        }
    }

    std::error_code existsError;
    const bool targetExists = std::filesystem::exists(fileName, existsError);
    if (existsError) {
        error = "无法检查现有数据文件：" + existsError.message();
        std::filesystem::remove(temporaryFile, errorCode);
        return false;
    }

    if (targetExists) {
        std::filesystem::remove(backupFile, errorCode);
        errorCode.clear();
        std::filesystem::rename(fileName, backupFile, errorCode);
        if (errorCode) {
            error = "无法备份现有数据文件：" + errorCode.message();
            std::filesystem::remove(temporaryFile, errorCode);
            return false;
        }
    }

    errorCode.clear();
    std::filesystem::rename(temporaryFile, fileName, errorCode);
    if (errorCode) {
        const std::string renameError = errorCode.message();
        std::error_code restoreError;
        if (std::filesystem::exists(backupFile, restoreError)) {
            std::filesystem::rename(backupFile, fileName, restoreError);
        }
        std::filesystem::remove(temporaryFile, errorCode);
        error = "无法替换数据文件：" + renameError;
        return false;
    }

    return true;
}

bool saveTasks(const std::vector<Task>& tasks, const std::filesystem::path& fileName) {
    std::string error;
    return saveTasksChecked(tasks, fileName, error);
}
