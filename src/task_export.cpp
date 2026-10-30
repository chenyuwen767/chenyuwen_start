#include "task_export.h"

namespace {

std::string escapeCsv(const std::string& value) {
    if (value.find_first_of(",\"\r\n") == std::string::npos) {
        return value;
    }

    std::string escaped = "\"";
    for (char character : value) {
        if (character == '"') {
            escaped += "\"\"";
        } else {
            escaped += character;
        }
    }
    escaped += '"';
    return escaped;
}

} // namespace

std::string tasksToCsv(const std::vector<Task>& tasks) {
    // UTF-8 BOM so Excel opens Chinese text without manual encoding selection.
    std::string csv = "\xEF\xBB\xBF题目,分类,难度,状态\n";
    for (const Task& task : tasks) {
        csv += escapeCsv(task.title);
        csv += ',';
        csv += escapeCsv(task.category);
        csv += ',';
        csv += escapeCsv(task.difficulty);
        csv += ',';
        csv += task.completed ? "已完成" : "待完成";
        csv += '\n';
    }
    return csv;
}
