#pragma once

#include <QDir>
#include <QString>
#include <QUuid>

class TempDir {
public:
    TempDir() {
        path_ = QDir::tempPath() + "/task_store_test_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
        valid_ = QDir().mkpath(path_);
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
