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

bool isLegacyHeader(const std::vector<std::string>& row) {
    return row.size() >= 4 && row[0] == "题目" && row[1] == "分类" &&
           row[2] == "难度" && row[3] == "状态";
}

bool isExtendedHeader(const std::vector<std::string>& row) {
    return row.size() >= 6 && row[0] == "题目" && row[1] == "分类" &&
           row[2] == "难度" && row[3] == "标签" && row[4] == "备注" &&
           row[5] == "状态";
}

std::vector<std::string> splitTags(const std::string& value) {
    std::vector<std::string> tags;
    std::size_t start = 0;
    while (start <= value.size()) {
        const std::size_t end = value.find(';', start);
        std::string tag = end == std::string::npos ? value.substr(start)
                                                   : value.substr(start, end - start);
        const std::size_t first = tag.find_first_not_of(" \t");
        const std::size_t last = tag.find_last_not_of(" \t");
        if (first != std::string::npos) {
            tags.push_back(tag.substr(first, last - first + 1));
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return tags;
}

std::string joinTags(const std::vector<std::string>& tags) {
    std::string joined;
    for (const std::string& tag : tags) {
        if (!joined.empty()) {
            joined += ';';
        }
        joined += tag;
    }
    return joined;
}

} // namespace

std::string tasksToCsv(const std::vector<Task>& tasks) {
    // UTF-8 BOM so Excel opens Chinese text without manual encoding selection.
    std::string csv = "\xEF\xBB\xBF题目,分类,难度,标签,备注,状态\n";
    for (const Task& task : tasks) {
        csv += escapeCsv(task.title);
        csv += ',';
        csv += escapeCsv(task.category);
        csv += ',';
        csv += escapeCsv(task.difficulty);
        csv += ',';
        csv += escapeCsv(joinTags(task.tags));
        csv += ',';
        csv += escapeCsv(task.notes);
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

    bool extended = false;
    std::size_t firstDataRow = 0;
    const std::vector<std::string>& firstRow = rows.front();
    if (isExtendedHeader(firstRow)) {
        extended = true;
        firstDataRow = 1;
    } else if (isLegacyHeader(firstRow)) {
        firstDataRow = 1;
    } else if (firstRow.size() >= 6) {
        extended = true;
    }

    std::vector<Task> parsed;
    for (std::size_t index = firstDataRow; index < rows.size(); ++index) {
        const std::vector<std::string>& row = rows[index];
        if (row.size() == 1 && row[0].empty()) {
            continue;
        }

        const std::size_t required = extended ? 6 : 4;
        if (row.size() < required) {
            error = "CSV 第 " + std::to_string(index + 1) + " 行字段不足";
            return false;
        }

        Task task;
        task.title = row[0];
        task.category = row[1];
        task.difficulty = row[2];
        if (extended) {
            task.tags = splitTags(row[3]);
            task.notes = row[4];
            if (!parseCompleted(row[5], task.completed)) {
                error = "CSV 第 " + std::to_string(index + 1) + " 行的状态无法识别";
                return false;
            }
        } else if (!parseCompleted(row[3], task.completed)) {
            error = "CSV 第 " + std::to_string(index + 1) + " 行的状态无法识别";
            return false;
        }
        parsed.push_back(std::move(task));
    }

    tasks = std::move(parsed);
    return true;
}
