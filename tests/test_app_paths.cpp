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
    void portableRestoresBackup();
    void reportsLegacyFromApplicationDirectory();
    void reportsLegacyFromCurrentDirectory();
    void restoresBackup();
    void keepsExistingFile();
    void normalDataDirectoryHasNoWarning();
    void fallsBackWhenDataDirectoryIsUnavailable();
    void prefersApplicationLegacyWhenAppDataIsUnavailable();
};

void AppPathsTest::portableUsesApplicationDirectory() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;

    QVERIFY(writeFile(applicationDirectory.filePath("tasks.json"), "{\"version\":2}"));

    const auto info = app_paths::resolveDataFileInfo(
        {"--portable"},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(info.path), applicationDirectory.filePath("tasks.json"));
    QVERIFY(info.legacyPath.empty());
    QVERIFY(info.warning.isEmpty());
    QVERIFY(!QFile::exists(dataDirectory.filePath("tasks.json")));
}

void AppPathsTest::portableRestoresBackup() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;
    QVERIFY(writeFile(applicationDirectory.filePath("tasks.json.bak"), "portable-backup"));

    const auto info = app_paths::resolveDataFileInfo(
        {"--portable"},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(info.path), applicationDirectory.filePath("tasks.json"));
    QCOMPARE(readFile(applicationDirectory.filePath("tasks.json")), QByteArray("portable-backup"));
}

void AppPathsTest::reportsLegacyFromApplicationDirectory() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;
    QVERIFY(writeFile(applicationDirectory.filePath("tasks.txt"), "from-app"));

    const auto info = app_paths::resolveDataFileInfo(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(info.path), dataDirectory.filePath("tasks.json"));
    QCOMPARE(pathOf(info.legacyPath), applicationDirectory.filePath("tasks.txt"));
    QVERIFY(info.warning.isEmpty());
    QVERIFY(!QFile::exists(dataDirectory.filePath("tasks.json")));
}

void AppPathsTest::reportsLegacyFromCurrentDirectory() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;
    QVERIFY(writeFile(currentDirectory.filePath("tasks.txt"), "from-cwd"));

    const auto info = app_paths::resolveDataFileInfo(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(info.path), dataDirectory.filePath("tasks.json"));
    QCOMPARE(pathOf(info.legacyPath), currentDirectory.filePath("tasks.txt"));
}

void AppPathsTest::restoresBackup() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;
    QVERIFY(writeFile(applicationDirectory.filePath("tasks.txt"), "stale-app"));
    QVERIFY(writeFile(dataDirectory.filePath("tasks.json.bak"), "backup"));

    const auto info = app_paths::resolveDataFileInfo(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(info.path), dataDirectory.filePath("tasks.json"));
    QCOMPARE(readFile(dataDirectory.filePath("tasks.json")), QByteArray("backup"));
    QVERIFY(info.legacyPath.empty());
}

void AppPathsTest::keepsExistingFile() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;
    QVERIFY(writeFile(applicationDirectory.filePath("tasks.txt"), "legacy"));
    QVERIFY(writeFile(dataDirectory.filePath("tasks.json"), "current"));

    const auto info = app_paths::resolveDataFileInfo(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(info.path), dataDirectory.filePath("tasks.json"));
    QCOMPARE(readFile(dataDirectory.filePath("tasks.json")), QByteArray("current"));
    QVERIFY(info.legacyPath.empty());
}

void AppPathsTest::normalDataDirectoryHasNoWarning() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    TempDir dataDirectory;

    const auto info = app_paths::resolveDataFileInfo(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        dataDirectory.path());

    QCOMPARE(pathOf(info.path), dataDirectory.filePath("tasks.json"));
    QVERIFY(info.warning.isEmpty());
    QVERIFY(info.legacyPath.empty());
}

void AppPathsTest::fallsBackWhenDataDirectoryIsUnavailable() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    QVERIFY(writeFile(applicationDirectory.filePath("blocked"), "not-a-directory"));

    const auto info = app_paths::resolveDataFileInfo(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        applicationDirectory.filePath("blocked"));

    QCOMPARE(pathOf(info.path), applicationDirectory.filePath("tasks.json"));
    QVERIFY(!info.warning.isEmpty());
}

void AppPathsTest::prefersApplicationLegacyWhenAppDataIsUnavailable() {
    TempDir applicationDirectory;
    TempDir currentDirectory;
    QVERIFY(writeFile(applicationDirectory.filePath("tasks.txt"), "from-app"));
    QVERIFY(writeFile(applicationDirectory.filePath("blocked"), "not-a-directory"));

    const auto info = app_paths::resolveDataFileInfo(
        {},
        applicationDirectory.path(),
        currentDirectory.path(),
        applicationDirectory.filePath("blocked"));

    QCOMPARE(pathOf(info.path), applicationDirectory.filePath("tasks.json"));
    QCOMPARE(pathOf(info.legacyPath), applicationDirectory.filePath("tasks.txt"));
}

int main(int argc, char** argv) {
    AppPathsTest instance;
    return QTest::qExec(&instance, argc, argv);
}

#include "test_app_paths.moc"
