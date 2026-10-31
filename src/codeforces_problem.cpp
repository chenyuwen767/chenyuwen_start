#include "codeforces_problem.h"

#include <QRegularExpression>

QString normalizeCodeforcesProblemId(const QString& input) {
    QString compact = input.trimmed().toUpper();
    compact.remove(QRegularExpression("\\s+"));
    compact.remove('-');
    compact.remove('_');
    if (compact.startsWith("CF")) {
        compact.remove(0, 2);
    }

    const QRegularExpression pattern("^(\\d+)([A-Z]\\d*)$");
    const QRegularExpressionMatch match = pattern.match(compact);
    if (!match.hasMatch()) {
        return {};
    }
    return match.captured(1) + match.captured(2);
}

QString codeforcesProblemKey(int contestId, const QString& index) {
    const QString normalizedIndex = index.trimmed().toUpper();
    if (contestId <= 0 || normalizedIndex.isEmpty()) {
        return {};
    }

    const QRegularExpression pattern("^[A-Z]\\d*$");
    if (!pattern.match(normalizedIndex).hasMatch()) {
        return {};
    }
    return QString::number(contestId) + normalizedIndex;
}
