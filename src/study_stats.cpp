#include "study_stats.h"

#include <QDateTime>
#include <QSet>

namespace {

QString valueOrUntimed(const std::string& value) {
    const QString text = QString::fromUtf8(value.c_str()).trimmed();
    return text.isEmpty() ? QString("未标注") : text;
}

void addCount(QMap<QString, int>& counts, const QString& key) {
    counts[key] = counts.value(key) + 1;
}

} // namespace

StudyStats computeStudyStats(const std::vector<Task>& tasks, const QDate& today) {
    StudyStats stats;
    stats.total = static_cast<int>(tasks.size());
    const QDate weekStart = today.addDays(1 - today.dayOfWeek());
    const QDate monthStart(today.year(), today.month(), 1);
    QSet<QDate> completedDates;

    for (const Task& task : tasks) {
        addCount(stats.totalByPlatform, valueOrUntimed(task.platform));
        addCount(stats.totalByCategory, valueOrUntimed(task.category));
        addCount(stats.totalByDifficulty, valueOrUntimed(task.difficulty));

        if (!task.completed) {
            continue;
        }

        ++stats.completed;
        addCount(stats.completedByPlatform, valueOrUntimed(task.platform));
        addCount(stats.completedByCategory, valueOrUntimed(task.category));
        addCount(stats.completedByDifficulty, valueOrUntimed(task.difficulty));

        const QDateTime completedAt = QDateTime::fromString(
            QString::fromUtf8(task.completedAt.c_str()), Qt::ISODate);
        if (!completedAt.isValid()) {
            ++stats.untimedCompleted;
            continue;
        }

        const QDate date = completedAt.toLocalTime().date();
        if (date > today) {
            continue;
        }

        completedDates.insert(date);
        if (date == today) {
            ++stats.today;
        }
        if (date >= weekStart && date <= today) {
            ++stats.week;
        }
        if (date >= monthStart && date <= today) {
            ++stats.month;
        }

        for (int index = 0; index < 8; ++index) {
            const QDate start = weekStart.addDays(-7 * (7 - index));
            const QDate end = start.addDays(6);
            if (date >= start && date <= end) {
                ++stats.weekly[static_cast<std::size_t>(index)];
                break;
            }
        }
    }

    QDate cursor = today;
    while (completedDates.contains(cursor)) {
        ++stats.streak;
        cursor = cursor.addDays(-1);
    }
    return stats;
}
