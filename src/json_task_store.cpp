#include "json_task_store.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QSaveFile>
#include <QString>

#include <utility>

namespace {

QString qpath(const std::filesystem::path& path) {
    return QString::fromStdWString(path.wstring());
}

std::string toStd(const QString& value) {
    return value.toUtf8().toStdString();
}

} // namespace

JsonLoadResult loadTasksJson(const std::filesystem::path& fileName) {
    JsonLoadResult result;
    QFile file(qpath(fileName));
    if (!file.exists()) {
        return result;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        result.ok = false;
        result.error = toStd("无法打开 JSON 数据文件：" + file.errorString());
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        result.ok = false;
        result.error = "tasks.json 不是有效的 JSON：" + toStd(parseError.errorString());
        return result;
    }

    const QJsonObject root = document.object();
    const QJsonValue versionValue = root.value("version");
    const int version = versionValue.isDouble() ? versionValue.toInt() : 3;
    if (version < 2 || version > 3) {
        result.warnings.push_back("tasks.json 的版本不是 2/3，已按兼容方式读取。");
    }

    const QJsonValue tasksValue = root.value("tasks");
    if (!tasksValue.isArray()) {
        result.ok = false;
        result.error = "tasks.json 缺少 tasks 数组。";
        return result;
    }

    const QJsonArray tasksArray = tasksValue.toArray();
    int index = 0;
    for (const QJsonValue& value : tasksArray) {
        ++index;
        if (!value.isObject()) {
            result.warnings.push_back("第 " + std::to_string(index) + " 条任务不是对象，已跳过。");
            continue;
        }

        const QJsonObject object = value.toObject();
        const QString title = object.value("title").toString();
        if (title.trimmed().isEmpty()) {
            result.warnings.push_back("第 " + std::to_string(index) + " 条任务缺少标题，已跳过。");
            continue;
        }

        Task task;
        task.title = toStd(title);
        task.category = toStd(object.value("category").toString());
        task.difficulty = toStd(object.value("difficulty").toString());
        task.completed = object.value("completed").toBool(false);
        task.notes = toStd(object.value("notes").toString());
        task.id = toStd(object.value("id").toString());
        task.platform = toStd(object.value("platform").toString());
        task.problemId = toStd(object.value("problemId").toString());

        const QJsonValue tagsValue = object.value("tags");
        if (tagsValue.isArray()) {
            for (const QJsonValue& tagValue : tagsValue.toArray()) {
                if (tagValue.isString()) {
                    const std::string tag = toStd(tagValue.toString());
                    if (!tag.empty()) {
                        task.tags.push_back(tag);
                    }
                }
            }
        } else if (!tagsValue.isUndefined() && !tagsValue.isNull()) {
            result.warnings.push_back("第 " + std::to_string(index) + " 条任务的 tags 不是数组，已忽略。");
        }

        result.tasks.push_back(std::move(task));
    }

    return result;
}

bool saveTasksJson(const std::vector<Task>& tasks,
                   const std::filesystem::path& fileName,
                   std::string& error) {
    QJsonArray tasksArray;
    for (const Task& task : tasks) {
        QJsonObject object;
        object.insert("id", QString::fromUtf8(task.id.c_str()));
        object.insert("title", QString::fromUtf8(task.title.c_str()));
        object.insert("category", QString::fromUtf8(task.category.c_str()));
        object.insert("difficulty", QString::fromUtf8(task.difficulty.c_str()));
        object.insert("completed", task.completed);
        object.insert("notes", QString::fromUtf8(task.notes.c_str()));
        object.insert("platform", QString::fromUtf8(task.platform.c_str()));
        object.insert("problemId", QString::fromUtf8(task.problemId.c_str()));

        QJsonArray tagsArray;
        for (const std::string& tag : task.tags) {
            tagsArray.append(QString::fromUtf8(tag.c_str()));
        }
        object.insert("tags", tagsArray);
        tasksArray.append(object);
    }

    QJsonObject root;
    root.insert("version", 3);
    root.insert("tasks", tasksArray);
    const QByteArray data = QJsonDocument(root).toJson(QJsonDocument::Indented);

    const QString target = qpath(fileName);
    const QString backup = target + ".bak";
    if (QFileInfo::exists(target)) {
        QFile::remove(backup);
        if (!QFile::copy(target, backup)) {
            error = "无法备份现有 tasks.json。";
            return false;
        }
    }

    QSaveFile output(target);
    if (!output.open(QIODevice::WriteOnly)) {
        error = toStd("无法创建 tasks.json：" + output.errorString());
        return false;
    }
    if (output.write(data) != data.size() || !output.commit()) {
        error = toStd("写入 tasks.json 失败：" + output.errorString());
        return false;
    }
    return true;
}
