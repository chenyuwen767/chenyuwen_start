#include "app_paths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QUuid>

namespace {

QString jsonFileIn(const QString& directory) {
    return QDir(directory).filePath("tasks.json");
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

QString firstExistingFile(const QStringList& directories, const QString& fileName) {
    for (const QString& directory : directories) {
        if (directory.isEmpty()) {
            continue;
        }
        const QString candidate = QDir(directory).filePath(fileName);
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}

QStringList uniqueDirectories(const QStringList& directories) {
    QStringList unique;
    for (const QString& directory : directories) {
        if (!directory.isEmpty() && !unique.contains(directory)) {
            unique << directory;
        }
    }
    return unique;
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

    const QString target = jsonFileIn(dataDirectory);
    if (!QFileInfo::exists(target)) {
        const QString backup = target + ".bak";
        if (QFileInfo::exists(backup)) {
            copyIfPresent(backup, target);
        } else {
            QStringList legacyDirectories;
            legacyDirectories << dataDirectory << applicationDirectory << currentDirectory;
            if (!portable) {
                legacyDirectories << applicationDataDirectory;
            }
            const QString legacy =
                firstExistingFile(uniqueDirectories(legacyDirectories), "tasks.txt");
            if (!legacy.isEmpty()) {
                info.legacyPath = std::filesystem::path(legacy.toStdWString());
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
