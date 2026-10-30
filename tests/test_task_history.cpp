#include <QtTest>

#include <string>
#include <vector>

#include "task_history.h"

namespace {

QString titleOf(const Task& task) {
    return QString::fromUtf8(task.title.c_str());
}

} // namespace

class TaskHistoryTest : public QObject {
    Q_OBJECT

private slots:
    void emptyHistoryCannotUndo();
    void undoesInReverseOrder();
    void dropsOldestSnapshotsAtLimit();
    void clearRemovesSnapshots();
};

void TaskHistoryTest::emptyHistoryCannotUndo() {
    TaskHistory history;
    std::vector<Task> state;
    QVERIFY(!history.canUndo());
    QVERIFY(!history.undo(state));
}

void TaskHistoryTest::undoesInReverseOrder() {
    TaskHistory history;
    history.record({{"first", "cat", "diff", false}});
    history.record({{"second", "cat", "diff", true}});
    QVERIFY(history.canUndo());

    std::vector<Task> state;
    QVERIFY(history.undo(state));
    QCOMPARE(static_cast<int>(state.size()), 1);
    QCOMPARE(titleOf(state[0]), QString("second"));
    QCOMPARE(state[0].completed, true);

    QVERIFY(history.undo(state));
    QCOMPARE(titleOf(state[0]), QString("first"));
    QVERIFY(!history.canUndo());
}

void TaskHistoryTest::dropsOldestSnapshotsAtLimit() {
    TaskHistory history(2);
    history.record({{"a", "cat", "diff", false}});
    history.record({{"b", "cat", "diff", false}});
    history.record({{"c", "cat", "diff", false}});

    std::vector<Task> state;
    QVERIFY(history.undo(state));
    QCOMPARE(titleOf(state[0]), QString("c"));
    QVERIFY(history.undo(state));
    QCOMPARE(titleOf(state[0]), QString("b"));
    QVERIFY(!history.undo(state));
}

void TaskHistoryTest::clearRemovesSnapshots() {
    TaskHistory history;
    history.record({{"a", "cat", "diff", false}});
    history.clear();
    QVERIFY(!history.canUndo());

    std::vector<Task> state;
    QVERIFY(!history.undo(state));
}

int main(int argc, char** argv) {
    TaskHistoryTest instance;
    return QTest::qExec(&instance, argc, argv);
}

#include "test_task_history.moc"
