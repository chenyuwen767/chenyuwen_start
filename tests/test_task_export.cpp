#include <QtTest>

#include <string>

#include "task_export.h"

class TaskExportTest : public QObject {
    Q_OBJECT

private slots:
    void startsWithBomAndHeader();
    void escapesCommasQuotesAndNewlines();
    void writesCompletedStatus();
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

QTEST_APPLESS_MAIN(TaskExportTest)

#include "test_task_export.moc"
