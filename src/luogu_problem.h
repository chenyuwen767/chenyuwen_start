#pragma once

#include <QString>
#include <QUrl>

struct LuoguProblem {
    QString pid;
    QString name;
    int difficulty = 0;
    int totalSubmit = 0;
    int totalAccepted = 0;
    bool valid = false;
};

QString normalizeLuoguProblemId(const QString& input);
QUrl luoguProblemUrl(const QString& problemId);
QString luoguDifficultyLabel(int difficulty);
bool parseLuoguProblemHtml(const QString& html, LuoguProblem& problem, QString& error);
