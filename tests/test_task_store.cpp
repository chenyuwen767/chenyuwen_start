#include <QtTest>

#include <QDir>
#include <QFile>

#include <filesystem>
#include <string>

#include "task_store.h"
#include "temp_dir.h"

namespace {

std::filesystem::path taskPath(const TempDir& directory, const QString& name = "tasks.txt") {
    return std::filesystem::path(directory.filePath(name).toStdWString());
}

QString textOf(const std::string& value) {
    return QString::fromUtf8(value.c_str());
}

} // namespace

class TaskStoreTest : public QObject {
    Q_OBJECT

private slots:
    void roundTripPreservesSpecialCharacters();
    void writesOnePhysicalLinePerTask();
    void createsBackupOfPreviousVersion();
    void reportsMalformedLinesWithoutDroppingValidOnes();
    void loadsLegacyFileWithCrlf();
    void loadsLegacyBackslashesLiterally();
    void stripsUtf8Bom();
    void handlesUnicodePath();
    void missingFileIsNotAnError();
    void reportsDirectoryAsError();
    void saveFailsWhenParentDirectoryIsMissing();
};

void TaskStoreTest::roundTripPreservesSpecialCharacters() {
    TempDir directory;
    QVERIFY(directory.isValid());
    const auto path = taskPath(directory);

    const std::vector<Task> tasks = {
        {"A\tB\nC\\D 中文", "分类\tX", "普及-\n", false},
        {"普通题目", "动态规划", "提高+", true},
    };

    QVERIFY(saveTasks(tasks, path));

    const LoadResult loaded = loadTasksChecked(path);
    QVERIFY(loaded.ok);
    QVERIFY(loaded.warnings.empty());
    QCOMPARE(static_cast<int>(loaded.tasks.size()), 2);
    QCOMPARE(textOf(loaded.tasks[0].title), QString("A\tB\nC\\D 中文"));
    QCOMPARE(textOf(loaded.tasks[0].category), QString("分类\tX"));
    QCOMPARE(textOf(loaded.tasks[0].difficulty), QString("普及-\n"));
    QCOMPARE(loaded.tasks[0].completed, false);
    QCOMPARE(textOf(loaded.tasks[1].title), QString("普通题目"));
    QCOMPARE(loaded.tasks[1].completed, true);
}

void TaskStoreTest::writesOnePhysicalLinePerTask() {
    TempDir directory;
    QVERIFY(directory.isValid());
    const auto path = taskPath(directory);

    QVERIFY(saveTasks({{"Line1\nLine2", "cat", "diff", false}}, path));

    QFile file(QString::fromStdWString(path.wstring()));
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray content = file.readAll();
    QCOMPARE(content.count('\n'), 2);
    QVERIFY(content.startsWith("# algorithm-practice-notebook v2\n"));
    QVERIFY(content.contains("Line1\\nLine2"));
}

void TaskStoreTest::createsBackupOfPreviousVersion() {
    TempDir directory;
    QVERIFY(directory.isValid());
    const auto path = taskPath(directory);

    QVERIFY(saveTasks({{"first", "cat", "diff", false}}, path));
    QVERIFY(saveTasks({{"second", "cat", "diff", true}}, path));

    auto backupPath = path;
    backupPath += ".bak";
    QVERIFY(std::filesystem::exists(backupPath));

    const LoadResult backup = loadTasksChecked(backupPath);
    QVERIFY(backup.ok);
    QCOMPARE(static_cast<int>(backup.tasks.size()), 1);
    QCOMPARE(textOf(backup.tasks[0].title), QString("first"));

    const LoadResult current = loadTasksChecked(path);
    QVERIFY(current.ok);
    QCOMPARE(static_cast<int>(current.tasks.size()), 1);
    QCOMPARE(textOf(current.tasks[0].title), QString("second"));

    QVERIFY(saveTasks({{"third", "cat", "diff", false}}, path));
    const LoadResult rotatedBackup = loadTasksChecked(backupPath);
    QVERIFY(rotatedBackup.ok);
    QCOMPARE(static_cast<int>(rotatedBackup.tasks.size()), 1);
    QCOMPARE(textOf(rotatedBackup.tasks[0].title), QString("second"));
}

void TaskStoreTest::reportsMalformedLinesWithoutDroppingValidOnes() {
    TempDir directory;
    QVERIFY(directory.isValid());
    const auto path = taskPath(directory);

    QFile file(QString::fromStdWString(path.wstring()));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("good\tcat\tdiff\t1\nonly-two\tfields\n");
    file.close();

    const LoadResult loaded = loadTasksChecked(path);
    QVERIFY(loaded.ok);
    QCOMPARE(static_cast<int>(loaded.tasks.size()), 1);
    QCOMPARE(static_cast<int>(loaded.warnings.size()), 1);
    QCOMPARE(textOf(loaded.tasks[0].title), QString("good"));
    QCOMPARE(loaded.tasks[0].completed, true);
}

void TaskStoreTest::loadsLegacyFileWithCrlf() {
    TempDir directory;
    QVERIFY(directory.isValid());
    const auto path = taskPath(directory);

    QFile file(QString::fromStdWString(path.wstring()));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("P17633\t无\t普及-\t1\r\n");
    file.close();

    const LoadResult loaded = loadTasksChecked(path);
    QVERIFY(loaded.ok);
    QCOMPARE(static_cast<int>(loaded.tasks.size()), 1);
    QCOMPARE(textOf(loaded.tasks[0].title), QString("P17633"));
    QCOMPARE(textOf(loaded.tasks[0].category), QString("无"));
    QCOMPARE(textOf(loaded.tasks[0].difficulty), QString("普及-"));
    QCOMPARE(loaded.tasks[0].completed, true);
}

void TaskStoreTest::loadsLegacyBackslashesLiterally() {
    TempDir directory;
    QVERIFY(directory.isValid());
    const auto path = taskPath(directory);

    QFile file(QString::fromStdWString(path.wstring()));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("C:\\temp\\new\tcat\tdiff\t0\n");
    file.close();

    const LoadResult loaded = loadTasksChecked(path);
    QVERIFY(loaded.ok);
    QCOMPARE(static_cast<int>(loaded.tasks.size()), 1);
    QCOMPARE(textOf(loaded.tasks[0].title), QString("C:\\temp\\new"));
}

void TaskStoreTest::stripsUtf8Bom() {
    TempDir directory;
    QVERIFY(directory.isValid());
    const auto path = taskPath(directory);

    QFile file(QString::fromStdWString(path.wstring()));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QByteArray("\xEF\xBB\xBF") + "P17633\t无\t普及-\t0\n");
    file.close();

    const LoadResult loaded = loadTasksChecked(path);
    QVERIFY(loaded.ok);
    QCOMPARE(static_cast<int>(loaded.tasks.size()), 1);
    QCOMPARE(textOf(loaded.tasks[0].title), QString("P17633"));
}

void TaskStoreTest::handlesUnicodePath() {
    TempDir directory;
    QVERIFY(directory.isValid());

    const QString subdirectory = directory.filePath("中文目录");
    QVERIFY(QDir().mkpath(subdirectory));
    const auto path = std::filesystem::path((subdirectory + "/tasks.txt").toStdWString());

    QVERIFY(saveTasks({{"题目", "分类", "难度", false}}, path));

    const LoadResult loaded = loadTasksChecked(path);
    QVERIFY(loaded.ok);
    QCOMPARE(static_cast<int>(loaded.tasks.size()), 1);
    QCOMPARE(textOf(loaded.tasks[0].title), QString("题目"));
}

void TaskStoreTest::missingFileIsNotAnError() {
    TempDir directory;
    QVERIFY(directory.isValid());

    const LoadResult loaded = loadTasksChecked(taskPath(directory, "missing.txt"));
    QVERIFY(loaded.ok);
    QVERIFY(loaded.tasks.empty());
    QVERIFY(loaded.warnings.empty());
}

void TaskStoreTest::reportsDirectoryAsError() {
    TempDir directory;
    QVERIFY(directory.isValid());

    const LoadResult loaded =
        loadTasksChecked(std::filesystem::path(directory.path().toStdWString()));
    QVERIFY(!loaded.ok);
    QVERIFY(!loaded.error.empty());
}

void TaskStoreTest::saveFailsWhenParentDirectoryIsMissing() {
    TempDir directory;
    QVERIFY(directory.isValid());

    std::string error;
    const auto path = taskPath(directory, "no-such-directory/tasks.txt");
    QVERIFY(!saveTasksChecked({{"title", "cat", "diff", false}}, path, error));
    QVERIFY(!error.empty());
}

int main(int argc, char** argv) {
    TaskStoreTest instance;
    return QTest::qExec(&instance, argc, argv);
}

#include "test_task_store.moc"
