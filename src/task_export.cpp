#include "task_export.h"

#include <utility>

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

std::vector<std::vector<std::string>> parseRows(const std::string& csv, std::string& error) {
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> row;
    std::string field;
    bool inQuotes = false;

    for (std::size_t index = 0; index < csv.size(); ++index) {
        const char character = csv[index];
        if (inQuotes) {
            if (character == '"') {
                if (index + 1 < csv.size() && csv[index + 1] == '"') {
                    field += '"';
                    ++index;
                } else {
                    inQuotes = false;
                }
            } else {
                field += character;
            }
            continue;
        }

        switch (character) {
        case '"':
            inQuotes = true;
            break;
        case ',':
            row.push_back(field);
            field.clear();
            break;
        case '\n':
            row.push_back(field);
            rows.push_back(row);
            row.clear();
            field.clear();
            break;
        case '\r':
            break;
        default:
            field += character;
            break;
        }
    }

    if (inQuotes) {
        error = "CSV 中的引号没有闭合";
        return {};
    }

    if (!field.empty() || !row.empty()) {
        row.push_back(field);
        rows.push_back(row);
    }
    return rows;
}

bool parseCompleted(const std::string& value, bool& completed) {
    if (value == "1" || value == "true" || value == "已完成") {
        completed = true;
        return true;
    }
    if (value == "0" || value == "false" || value == "待完成") {
        completed = false;
        return true;
    }
    return false;
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

bool tasksFromCsv(const std::string& csv, std::vector<Task>& tasks, std::string& error) {
    std::string content = csv;
    if (content.rfind("\xEF\xBB\xBF", 0) == 0) {
        content.erase(0, 3);
    }

    const std::vector<std::vector<std::string>> rows = parseRows(content, error);
    if (!error.empty()) {
        return false;
    }
    if (rows.empty()) {
        error = "CSV 文件为空";
        return false;
    }

    const std::vector<std::string>& header = rows.front();
    const bool hasHeader = header.size() >= 4 && header[0] == "题目" && header[1] == "分类" &&
                           header[2] == "难度" && header[3] == "状态";
    const std::size_t firstDataRow = hasHeader ? 1 : 0;

    std::vector<Task> parsed;
    for (std::size_t index = firstDataRow; index < rows.size(); ++index) {
        const std::vector<std::string>& row = rows[index];
        if (row.size() == 1 && row[0].empty()) {
            continue;
        }
        if (row.size() < 4) {
            error = "CSV 第 " + std::to_string(index + 1) + " 行字段不足";
            return false;
        }

        bool completed = false;
        if (!parseCompleted(row[3], completed)) {
            error = "CSV 第 " + std::to_string(index + 1) + " 行的状态无法识别";
            return false;
        }
        parsed.push_back({row[0], row[1], row[2], completed});
    }

    tasks = std::move(parsed);
    return true;
}
