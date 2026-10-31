#pragma once

#include <QDate>
#include <QMap>
#include <QString>

#include <array>
#include <vector>

#include "task_store.h"

struct StudyStats {
    int total = 0;
    int completed = 0;
    int untimedCompleted = 0;
    int today = 0;
    int week = 0;
    int month = 0;
    int streak = 0;
    std::array<int, 8> weekly{};

    QMap<QString, int> totalByPlatform;
    QMap<QString, int> completedByPlatform;
    QMap<QString, int> totalByCategory;
    QMap<QString, int> completedByCategory;
    QMap<QString, int> totalByDifficulty;
    QMap<QString, int> completedByDifficulty;
};

StudyStats computeStudyStats(const std::vector<Task>& tasks, const QDate& today);
