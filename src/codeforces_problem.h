#pragma once

#include <QString>
#include <QUrl>

QString normalizeCodeforcesProblemId(const QString& input);
QString codeforcesProblemKey(int contestId, const QString& index);
QUrl codeforcesProblemUrl(const QString& problemId);
