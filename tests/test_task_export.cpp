#include <QtTest>

#include <string>

#include "task_export.h"

class TaskExportTest : public QObject {
    Q_OBJECT

private slots:
    void startsWithBomAndHeader();
    void escapesCommasQuotesAndNewlines();
    void writesCompletedStatus();
    void roundTripsThroughCsv();
    void importsLegacyFourColumnCsv();
    void importsVersion6Csv();
    void acceptsHeaderlessRows();
    void acceptsVersion8HeaderlessRows();
    void acceptsCrlfFromExcel();
    void rejectsMissingColumns();
    void rejectsUnknownStatus();
};

void TaskExportTest::startsWithBomAndHeader() {
    const std::string csv = tasksToCsv({});
    QVERIFY(csv.rfind("\xEF\xBB\xBF题目,分类,难度,平台,题号,标签,备注,状态\n", 0) == 0);
}

void TaskExportTest::escapesCommasQuotesAndNewlines() {
    Task task;
    task.title = "A,B";
    task.category = "C\"D";
    task.difficulty = "E\nF";
    task.platform = "codeforces";
    task.problemId = "1900A";
    task.completed = true;
    task.tags = {"t1", "t2"};
    task.notes = "notes, \"x\"\nline2";

    const std::string csv = tasksToCsv({task});
    QVERIFY(csv.find(
                "\"A,B\",\"C\"\"D\",\"E\nF\",codeforces,1900A,t1;t2,"
                "\"notes, \"\"x\"\"\nline2\",已完成\n") != std::string::npos);
}

void TaskExportTest::writesCompletedStatus() {
    Task task;
    task.title = "普通题目";
    task.category = "分类";
    task.difficulty = "难度";
    task.completed = false;

    const std::string csv = tasksToCsv({task});
    QVERIFY(csv.find("普通题目,分类,难度,,,,,待完成\n") != std::string::npos);
}

void TaskExportTest::roundTripsThroughCsv() {
    Task first;
    first.title = "A,B";
    first.category = "C\"D";
    first.difficulty = "E\nF";
    first.platform = "codeforces";
    first.problemId = "1900A";
    first.completed = true;
    first.tags = {"trie", "xor"};
    first.notes = "第一行\n第二行";

    Task second;
    second.title = "普通题目";
    second.category = "分类";
    second.difficulty = "难度";
    second.platform = "luogu";
    second.problemId = "P1001";
    second.completed = false;

    std::vector<Task> parsed;
    std::string error;
    QVERIFY(tasksFromCsv(tasksToCsv({first, second}), parsed, error));
    QVERIFY(error.empty());
    QCOMPARE(static_cast<int>(parsed.size()), 2);
    QCOMPARE(QString::fromUtf8(parsed[0].title.c_str()), QString("A,B"));
    QCOMPARE(QString::fromUtf8(parsed[0].platform.c_str()), QString("codeforces"));
    QCOMPARE(QString::fromUtf8(parsed[0].problemId.c_str()), QString("1900A"));
    QCOMPARE(QString::fromUtf8(parsed[0].notes.c_str()), QString("第一行\n第二行"));
    QCOMPARE(static_cast<int>(parsed[0].tags.size()), 2);
    QCOMPARE(QString::fromUtf8(parsed[0].tags[1].c_str()), QString("xor"));
    QCOMPARE(parsed[0].completed, true);
    QCOMPARE(QString::fromUtf8(parsed[1].platform.c_str()), QString("luogu"));
    QCOMPARE(QString::fromUtf8(parsed[1].problemId.c_str()), QString("P1001"));
    QCOMPARE(parsed[1].completed, false);
}

void TaskExportTest::importsLegacyFourColumnCsv() {
    std::vector<Task> parsed;
    std::string error;
    QVERIFY(tasksFromCsv("题目,分类,难度,状态\nA,B,C,已完成\n", parsed, error));
    QCOMPARE(static_cast<int>(parsed.size()), 1);
    QCOMPARE(QString::fromUtf8(parsed[0].title.c_str()), QString("A"));
    QVERIFY(parsed[0].tags.empty());
    QVERIFY(parsed[0].platform.empty());
    QCOMPARE(parsed[0].completed, true);
}

void TaskExportTest::importsVersion6Csv() {
    std::vector<Task> parsed;
    std::string error;
    QVERIFY(tasksFromCsv(
        "题目,分类,难度,标签,备注,状态\nA,B,C,t1;t2,note,待完成\n", parsed, error));
    QCOMPARE(static_cast<int>(parsed.size()), 1);
    QCOMPARE(static_cast<int>(parsed[0].tags.size()), 2);
    QCOMPARE(QString::fromUtf8(parsed[0].notes.c_str()), QString("note"));
    QVERIFY(parsed[0].platform.empty());
}

void TaskExportTest::acceptsHeaderlessRows() {
    std::vector<Task> parsed;
    std::string error;
    QVERIFY(tasksFromCsv("A,B,C,已完成\n", parsed, error));
    QCOMPARE(static_cast<int>(parsed.size()), 1);
    QCOMPARE(parsed[0].completed, true);
}

void TaskExportTest::acceptsVersion8HeaderlessRows() {
    std::vector<Task> parsed;
    std::string error;
    QVERIFY(tasksFromCsv("A,B,C,codeforces,1900A,t1;t2,note,待完成\n", parsed, error));
    QCOMPARE(static_cast<int>(parsed.size()), 1);
    QCOMPARE(static_cast<int>(parsed[0].tags.size()), 2);
    QCOMPARE(QString::fromUtf8(parsed[0].platform.c_str()), QString("codeforces"));
    QCOMPARE(QString::fromUtf8(parsed[0].notes.c_str()), QString("note"));
    QCOMPARE(parsed[0].completed, false);
}

void TaskExportTest::acceptsCrlfFromExcel() {
    std::vector<Task> parsed;
    std::string error;
    QVERIFY(tasksFromCsv(
        "题目,分类,难度,平台,题号,标签,备注,状态\r\nA,B,C,codeforces,1900A,t1,note,已完成\r\n",
        parsed,
        error));
    QCOMPARE(static_cast<int>(parsed.size()), 1);
    QCOMPARE(parsed[0].completed, true);
    QCOMPARE(static_cast<int>(parsed[0].tags.size()), 1);
}

void TaskExportTest::rejectsMissingColumns() {
    std::vector<Task> parsed;
    std::string error;
    QVERIFY(!tasksFromCsv("题目,分类\nA,B\n", parsed, error));
    QVERIFY(!error.empty());
}

void TaskExportTest::rejectsUnknownStatus() {
    std::vector<Task> parsed;
    std::string error;
    QVERIFY(!tasksFromCsv("题目,分类,难度,状态\nA,B,C,未知\n", parsed, error));
    QVERIFY(!error.empty());
}

int main(int argc, char** argv) {
    TaskExportTest instance;
    return QTest::qExec(&instance, argc, argv);
}

#include "test_task_export.moc"
