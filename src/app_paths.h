#pragma once

#include <QString>
#include <QStringList>

#include <filesystem>

namespace app_paths {

struct DataFileInfo {
    std::filesystem::path path;
    std::filesystem::path legacyPath;
    QString warning;
};

// Resolves tasks.json, creates the application data directory when needed,
// restores a backup if the main file is missing, and reports a legacy
// tasks.txt that should be migrated by the caller.
// If the preferred directory is not writable, a writable fallback is used and
// `warning` explains the change.
DataFileInfo resolveDataFileInfo(const QStringList& arguments,
                                 const QString& applicationDirectory,
                                 const QString& currentDirectory,
                                 const QString& applicationDataDirectory);

std::filesystem::path resolveDataFile(const QStringList& arguments,
                                      const QString& applicationDirectory,
                                      const QString& currentDirectory,
                                      const QString& applicationDataDirectory);

} // namespace app_paths
