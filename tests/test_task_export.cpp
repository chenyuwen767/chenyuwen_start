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
    void acceptsHeaderlessRows();
    void rejectsMissingColumns();
    void rejectsUnknownStatus();
};

void TaskExportTest::startsWithBomAndHeader() {
    const std::string csv = tasksToCsv({});
    QVERIFY(csv.rfind("\xEF\xBB\xBF题目,分类,难度,状态\n", 0) == 0);
}

void TaskExportTest::escapesCommasQuotesAndNewlines() {
    const std::string csv = tasksToCsv({{"A,B", "C\"D", "E\nF", true}});
    QVERIFY(csv.find("\"A,B\",\"C\"\"D\",\"E\nF\",已完成\n") != std::string::npos);
}

void TaskExportTest::writesCompletedStatus() {
    const std::string csv = tasksToCsv({{"普通题目", "分类", "难度", false}});
    QVERIFY(csv.find("普通题目,分类,难度,待完成\n") != std::string::npos);
}

void TaskExportTest::roundTripsThroughCsv() {
    const std::vector<Task> original = {
        {"A,B", "C\"D", "E\nF", true},
        {"普通题目", "分类", "难度", false},
    };

    std::vector<Task> parsed;
    std::string error;
    QVERIFY(tasksFromCsv(tasksToCsv(original), parsed, error));
    QVERIFY(error.empty());
    QCOMPARE(static_cast<int>(parsed.size()), 2);
    QCOMPARE(QString::fromUtf8(parsed[0].title.c_str()), QString("A,B"));
    QCOMPARE(QString::fromUtf8(parsed[0].category.c_str()), QString("C\"D"));
    QCOMPARE(QString::fromUtf8(parsed[0].difficulty.c_str()), QString("E\nF"));
    QCOMPARE(parsed[0].completed, true);
    QCOMPARE(QString::fromUtf8(parsed[1].title.c_str()), QString("普通题目"));
    QCOMPARE(parsed[1].completed, false);
}

void TaskExportTest::acceptsHeaderlessRows() {
    std::vector<Task> parsed;
    std::string error;
    QVERIFY(tasksFromCsv("A,B,C,已完成\n", parsed, error));
    QCOMPARE(static_cast<int>(parsed.size()), 1);
    QCOMPARE(parsed[0].completed, true);
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
