#pragma once

#include <QString>
#include <QStringList>

#include <filesystem>

namespace app_paths {

// Resolves the task data file, creates the application data directory when
// needed, restores a backup if the main file is missing, and migrates a legacy
// tasks.txt from the application directory or the current directory.
std::filesystem::path resolveDataFile(const QStringList& arguments,
                                      const QString& applicationDirectory,
                                      const QString& currentDirectory,
                                      const QString& applicationDataDirectory);

} // namespace app_paths
