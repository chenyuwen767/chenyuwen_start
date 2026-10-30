#include "app_paths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

namespace {

QString taskFileIn(const QString& directory) {
    return QDir(directory).filePath("tasks.txt");
}

void copyIfPresent(const QString& source, const QString& target) {
    if (QFileInfo::exists(source)) {
        QFile::copy(source, target);
    }
}

bool isWritableDirectory(const QString& directory) {
    QDir dir(directory);
    if (!dir.exists()) {
        return false;
    }

    const QString probe =
        dir.filePath(".algorithm_notebook_write_probe_" +
                     QUuid::createUuid().toString(QUuid::WithoutBraces));
    QFile file(probe);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.close();
    QFile::remove(probe);
    return true;
}

} // namespace

app_paths::DataFileInfo app_paths::resolveDataFileInfo(const QStringList& arguments,
                                                       const QString& applicationDirectory,
                                                       const QString& currentDirectory,
                                                       const QString& applicationDataDirectory) {
    DataFileInfo info;
    const bool portable = arguments.contains("--portable");

    QString dataDirectory;
    if (portable) {
        dataDirectory = applicationDirectory;
        if (!isWritableDirectory(dataDirectory)) {
            dataDirectory = QDir::tempPath();
            info.warning = "程序目录不可写，已改用临时目录；任务数据可能不会长期保留。";
        }
    } else {
        dataDirectory = applicationDataDirectory;
        if (!QDir().mkpath(dataDirectory) || !isWritableDirectory(dataDirectory)) {
            if (isWritableDirectory(applicationDirectory)) {
                dataDirectory = applicationDirectory;
                info.warning = "应用数据目录不可写，已改用程序所在目录。";
            } else {
                dataDirectory = QDir::tempPath();
                info.warning = "应用数据目录和程序目录都不可写，已改用临时目录；任务数据可能不会长期保留。";
            }
        }
    }

    const QString target = taskFileIn(dataDirectory);
    if (!QFileInfo::exists(target)) {
        const QString backup = target + ".bak";
        if (QFileInfo::exists(backup)) {
            copyIfPresent(backup, target);
        } else if (!portable) {
            copyIfPresent(taskFileIn(applicationDirectory), target);
            if (!QFileInfo::exists(target)) {
                copyIfPresent(taskFileIn(currentDirectory), target);
            }
        }
    }

    info.path = std::filesystem::path(target.toStdWString());
    return info;
}

std::filesystem::path app_paths::resolveDataFile(const QStringList& arguments,
                                                  const QString& applicationDirectory,
                                                  const QString& currentDirectory,
                                                  const QString& applicationDataDirectory) {
    return resolveDataFileInfo(arguments, applicationDirectory, currentDirectory,
                               applicationDataDirectory)
        .path;
}
