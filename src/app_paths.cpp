#include "app_paths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace {

QString taskFileIn(const QString& directory) {
    return QDir(directory).filePath("tasks.txt");
}

void copyIfPresent(const QString& source, const QString& target) {
    if (QFileInfo::exists(source)) {
        QFile::copy(source, target);
    }
}

} // namespace

std::filesystem::path app_paths::resolveDataFile(const QStringList& arguments,
                                                  const QString& applicationDirectory,
                                                  const QString& currentDirectory,
                                                  const QString& applicationDataDirectory) {
    const bool portable = arguments.contains("--portable");
    QString dataDirectory = applicationDataDirectory;
    if (!portable && !QDir().mkpath(dataDirectory)) {
        dataDirectory = applicationDirectory;
    }
    const QString target = taskFileIn(portable ? applicationDirectory : dataDirectory);

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

    return std::filesystem::path(target.toStdWString());
}
