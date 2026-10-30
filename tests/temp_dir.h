#pragma once

#include <QDir>
#include <QString>
#include <QStringList>
#include <QUuid>

class TempDir {
public:
    TempDir() {
        const QString suffix = "task_store_test_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
        const QStringList bases = {QDir::currentPath(), QDir::tempPath()};
        for (const QString& base : bases) {
            if (base.isEmpty()) {
                continue;
            }
            path_ = base + "/" + suffix;
            if (QDir().mkpath(path_)) {
                valid_ = true;
                break;
            }
        }
    }

    ~TempDir() {
        if (valid_) {
            QDir(path_).removeRecursively();
        }
    }

    bool isValid() const { return valid_; }
    QString path() const { return path_; }
    QString filePath(const QString& name) const { return path_ + "/" + name; }

private:
    QString path_;
    bool valid_ = false;
};
