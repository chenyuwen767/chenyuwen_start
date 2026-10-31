#include <QtTest>

#include <QCoreApplication>

#include <string>
#include <vector>

#include "study_stats.h"

class StudyStatsTest : public QObject {
    Q_OBJECT

private slots:
    void computesTotalsAndPeriods();
    void computesStreakAndBreaks();
    void computesWeeklyBuckets();
    void computesDistributions();
    void ignoresInvalidAndFutureTimestamps();
};

namespace {

Task completedTask(const QString& completedAt, const QString& platform = QString()) {
    Task task;
    task.title = "task";
    task.category = "基础";
    task.difficulty = "800";
    task.completed = true;
    task.platform = platform.toStdString();
    task.completedAt = completedAt.toStdString();
    return task;
}

} // namespace

void StudyStatsTest::computesTotalsAndPeriods() {
    std::vector<Task> tasks;
    tasks.push_back(completedTask("2026-10-31T10:00:00+08:00", "codeforces"));
    tasks.push_back(completedTask("2026-10-30T10:00:00+08:00", "luogu"));
    tasks.push_back(completedTask("2026-10-01T10:00:00+08:00"));
    tasks.push_back(completedTask(""));
    tasks.push_back(Task{"pending", "基础", "800", false});

    const StudyStats stats = computeStudyStats(tasks, QDate(2026, 10, 31));
    QCOMPARE(stats.total, 5);
    QCOMPARE(stats.completed, 4);
    QCOMPARE(stats.untimedCompleted, 1);
    QCOMPARE(stats.today, 1);
    QCOMPARE(stats.week, 2);
    QCOMPARE(stats.month, 3);
}

void StudyStatsTest::computesStreakAndBreaks() {
    std::vector<Task> tasks;
    tasks.push_back(completedTask("2026-10-31T10:00:00+08:00"));
    tasks.push_back(completedTask("2026-10-30T10:00:00+08:00"));
    tasks.push_back(completedTask("2026-10-29T10:00:00+08:00"));

    QCOMPARE(computeStudyStats(tasks, QDate(2026, 10, 31)).streak, 3);

    std::vector<Task> noToday;
    noToday.push_back(completedTask("2026-10-30T10:00:00+08:00"));
    QCOMPARE(computeStudyStats(noToday, QDate(2026, 10, 31)).streak, 0);

    std::vector<Task> gap;
    gap.push_back(completedTask("2026-10-31T10:00:00+08:00"));
    gap.push_back(completedTask("2026-10-29T10:00:00+08:00"));
    QCOMPARE(computeStudyStats(gap, QDate(2026, 10, 31)).streak, 1);
}

void StudyStatsTest::computesWeeklyBuckets() {
    std::vector<Task> tasks;
    tasks.push_back(completedTask("2026-10-31T10:00:00+08:00"));
    tasks.push_back(completedTask("2026-10-26T10:00:00+08:00"));
    tasks.push_back(completedTask("2026-10-19T10:00:00+08:00"));
    tasks.push_back(completedTask("2026-09-28T10:00:00+08:00"));

    const StudyStats stats = computeStudyStats(tasks, QDate(2026, 10, 31));
    QCOMPARE(stats.weekly[7], 2);
    QCOMPARE(stats.weekly[6], 1);
    QCOMPARE(stats.weekly[5], 0);
    QCOMPARE(stats.weekly[4], 0);
    QCOMPARE(stats.weekly[3], 1);
    QCOMPARE(stats.weekly[0], 0);
}

void StudyStatsTest::computesDistributions() {
    std::vector<Task> tasks;
    tasks.push_back(completedTask("2026-10-31T10:00:00+08:00", "codeforces"));
    tasks.push_back(completedTask("2026-10-30T10:00:00+08:00", "luogu"));
    tasks.push_back(completedTask("", "luogu"));

    const StudyStats stats = computeStudyStats(tasks, QDate(2026, 10, 31));
    QCOMPARE(stats.totalByPlatform.value("codeforces"), 1);
    QCOMPARE(stats.totalByPlatform.value("luogu"), 2);
    QCOMPARE(stats.completedByPlatform.value("luogu"), 2);
    QCOMPARE(stats.totalByCategory.value("基础"), 3);
    QCOMPARE(stats.totalByDifficulty.value("800"), 3);
}

void StudyStatsTest::ignoresInvalidAndFutureTimestamps() {
    std::vector<Task> tasks;
    tasks.push_back(completedTask("not-a-date"));
    tasks.push_back(completedTask("2027-01-01T10:00:00+08:00"));

    const StudyStats stats = computeStudyStats(tasks, QDate(2026, 10, 31));
    QCOMPARE(stats.completed, 2);
    QCOMPARE(stats.untimedCompleted, 1);
    QCOMPARE(stats.today, 0);
    QCOMPARE(stats.streak, 0);
}

int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);
    StudyStatsTest instance;
    return QTest::qExec(&instance, argc, argv);
}

#include "test_study_stats.moc"
