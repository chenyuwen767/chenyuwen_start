#include <QtTest>

#include <QCoreApplication>

#include <string>
#include <utility>
#include <vector>

#include "task_table_model.h"

class TaskTableModelTest : public QObject {
    Q_OBJECT

private slots:
    void exposesColumnsAndRows();
    void filtersByKeywordAcrossFields();
    void filtersPendingOnly();
    void sortsByTitle();
    void exposesNotesAsToolTip();
};

namespace {

std::vector<Task> sampleTasks() {
    Task alpha;
    alpha.title = "Alpha";
    alpha.category = "dp";
    alpha.difficulty = "800";
    alpha.tags = {"dp", "graph"};
    alpha.notes = "graph note";
    alpha.completed = false;
    alpha.platform = "codeforces";
    alpha.problemId = "1900A";

    Task beta;
    beta.title = "Beta";
    beta.category = "math";
    beta.difficulty = "1200";
    beta.completed = true;
    beta.platform = "luogu";
    beta.problemId = "P1001";

    return {alpha, beta};
}

} // namespace

void TaskTableModelTest::exposesColumnsAndRows() {
    std::vector<Task> tasks = sampleTasks();
    TaskTableModel model;
    model.setTasks(&tasks);

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.columnCount(), static_cast<int>(TaskTableModel::ColumnCount));
    QCOMPARE(model.headerData(TaskTableModel::TitleColumn, Qt::Horizontal).toString(),
             QString("题目名称"));
    QCOMPARE(model.headerData(TaskTableModel::TagsColumn, Qt::Horizontal).toString(),
             QString("标签"));
    QCOMPARE(model.data(model.index(0, TaskTableModel::TitleColumn)).toString(), QString("Alpha"));
    QCOMPARE(model.data(model.index(0, TaskTableModel::ProblemColumn)).toString(),
             QString("CF 1900A"));
    QCOMPARE(model.data(model.index(1, TaskTableModel::ProblemColumn)).toString(),
             QString("洛谷 P1001"));
    QCOMPARE(model.data(model.index(0, TaskTableModel::TagsColumn)).toString(),
             QString("dp, graph"));
    QCOMPARE(model.data(model.index(1, TaskTableModel::StatusColumn)).toString(),
             QString("已完成"));
    QVERIFY(model.taskAt(0) != nullptr);
    QVERIFY(model.taskAt(2) == nullptr);
}

void TaskTableModelTest::filtersByKeywordAcrossFields() {
    std::vector<Task> tasks = sampleTasks();
    TaskTableModel model;
    model.setTasks(&tasks);
    TaskFilterProxyModel proxy;
    proxy.setSourceModel(&model);

    proxy.setKeyword("dp");
    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.data(proxy.index(0, TaskTableModel::TitleColumn)).toString(), QString("Alpha"));

    proxy.setKeyword("graph note");
    QCOMPARE(proxy.rowCount(), 1);

    proxy.setKeyword("1900A");
    QCOMPARE(proxy.rowCount(), 1);

    proxy.setKeyword("P1001");
    QCOMPARE(proxy.rowCount(), 1);

    proxy.setKeyword("does-not-exist");
    QCOMPARE(proxy.rowCount(), 0);
}

void TaskTableModelTest::filtersPendingOnly() {
    std::vector<Task> tasks = sampleTasks();
    TaskTableModel model;
    model.setTasks(&tasks);
    TaskFilterProxyModel proxy;
    proxy.setSourceModel(&model);

    proxy.setPendingOnly(true);
    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.data(proxy.index(0, TaskTableModel::TitleColumn)).toString(), QString("Alpha"));

    proxy.setPendingOnly(false);
    QCOMPARE(proxy.rowCount(), 2);
}

void TaskTableModelTest::sortsByTitle() {
    std::vector<Task> tasks = sampleTasks();
    std::swap(tasks[0], tasks[1]);

    TaskTableModel model;
    model.setTasks(&tasks);
    TaskFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    proxy.sort(TaskTableModel::TitleColumn, Qt::AscendingOrder);

    QCOMPARE(proxy.data(proxy.index(0, TaskTableModel::TitleColumn)).toString(), QString("Alpha"));
    QCOMPARE(proxy.data(proxy.index(1, TaskTableModel::TitleColumn)).toString(), QString("Beta"));
}

void TaskTableModelTest::exposesNotesAsToolTip() {
    std::vector<Task> tasks = sampleTasks();
    TaskTableModel model;
    model.setTasks(&tasks);

    QCOMPARE(model.data(model.index(0, TaskTableModel::TitleColumn), Qt::ToolTipRole).toString(),
             QString("graph note"));
}

int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);
    TaskTableModelTest instance;
    return QTest::qExec(&instance, argc, argv);
}

#include "test_task_table_model.moc"
