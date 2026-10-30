#include <QtTest>

#include <QFile>

#include "app_paths.h"
#include "temp_dir.h"

namespace {

bool writeFile(const QString& path, const QByteArray& content) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    return file.write(content) == content.size();
}

QByteArray readFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}

QString pathOf(const std::filesystem::path& path) {
    return QString::fromStdWString(path.wstring());
}

} // namespace

class AppPathsTest : public QObject {
    Q_OBJECT

private slots:
    void portableUsesApplicationDirectory();
    void migratesFromApplicationDirectory();
    void migratesFromCurrentDirectory();
    void restoresBackup();
    void keepsExistingFile();
    void fallsBackWhenDataDirectoryIsUnavailable();
};

void AppPathsTest::portableUsesApplicationDirectory() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;
    QVERIFY(applicationDirectory.isValid());
    QVERIFY(currentDirectory.isValid());
    QVERIFY(dataDirectory.isValid());

    QVERIFY(writeFile(applicationDirectory.filePath("tasks.txt"), "portable"));

    const auto resolved = app_paths::resolveDataFile(
        {"--portable"},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(resolved), applicationDirectory.filePath("tasks.txt"));
    QVERIFY(!QFile::exists(dataDirectory.filePath("tasks.txt")));
}

void AppPathsTest::migratesFromApplicationDirectory() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;
    QVERIFY(writeFile(applicationDirectory.filePath("tasks.txt"), "from-app"));

    const auto resolved = app_paths::resolveDataFile(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(resolved), dataDirectory.filePath("tasks.txt"));
    QCOMPARE(readFile(dataDirectory.filePath("tasks.txt")), QByteArray("from-app"));
}

void AppPathsTest::migratesFromCurrentDirectory() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;
    QVERIFY(writeFile(currentDirectory.filePath("tasks.txt"), "from-cwd"));

    const auto resolved = app_paths::resolveDataFile(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(resolved), dataDirectory.filePath("tasks.txt"));
    QCOMPARE(readFile(dataDirectory.filePath("tasks.txt")), QByteArray("from-cwd"));
}

void AppPathsTest::restoresBackup() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;
    QVERIFY(writeFile(applicationDirectory.filePath("tasks.txt"), "stale-app"));
    QVERIFY(writeFile(dataDirectory.filePath("tasks.txt.bak"), "backup"));

    const auto resolved = app_paths::resolveDataFile(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(resolved), dataDirectory.filePath("tasks.txt"));
    QCOMPARE(readFile(dataDirectory.filePath("tasks.txt")), QByteArray("backup"));
}

void AppPathsTest::keepsExistingFile() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;
    QVERIFY(writeFile(applicationDirectory.filePath("tasks.txt"), "legacy"));
    QVERIFY(writeFile(dataDirectory.filePath("tasks.txt"), "current"));

    const auto resolved = app_paths::resolveDataFile(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(resolved), dataDirectory.filePath("tasks.txt"));
    QCOMPARE(readFile(dataDirectory.filePath("tasks.txt")), QByteArray("current"));
}

void AppPathsTest::fallsBackWhenDataDirectoryIsUnavailable() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    QVERIFY(writeFile(applicationDirectory.filePath("blocked"), "not-a-directory"));

    const auto resolved = app_paths::resolveDataFile(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        applicationDirectory.filePath("blocked"));

    QCOMPARE(pathOf(resolved), applicationDirectory.filePath("tasks.txt"));
}

QTEST_APPLESS_MAIN(AppPathsTest)

#include "test_app_paths.moc"
