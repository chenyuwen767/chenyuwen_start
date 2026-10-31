#include "luogu_problem.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

QString normalizeLuoguProblemId(const QString& input) {
    QString value = input.trimmed();
    if (value.contains("/problem/")) {
        value = value.section("/problem/", 1).section('/', 0, 0).section('?', 0, 0);
    }
    value = value.trimmed();
    while (value.endsWith('/')) {
        value.chop(1);
    }

    static const QRegularExpression pattern("^[A-Za-z][A-Za-z0-9_]{0,31}$");
    if (!pattern.match(value).hasMatch()) {
        return {};
    }
    return value;
}

QUrl luoguProblemUrl(const QString& problemId) {
    const QString normalized = normalizeLuoguProblemId(problemId);
    if (normalized.isEmpty()) {
        return {};
    }

    QUrl url;
    url.setScheme("https");
    url.setHost("www.luogu.com.cn");
    url.setPath("/problem/" + normalized);
    return url;
}

QString luoguDifficultyLabel(int difficulty) {
    switch (difficulty) {
    case 0:
        return "暂无评定";
    case 1:
        return "入门";
    case 2:
        return "普及-";
    case 3:
        return "普及/提高-";
    case 4:
        return "普及+/提高";
    case 5:
        return "提高+/省选-";
    case 6:
        return "省选/NOI-";
    case 7:
        return "NOI/NOI+/CTSC";
    default:
        return "未评定";
    }
}

bool parseLuoguProblemHtml(const QString& html, LuoguProblem& problem, QString& error) {
    problem = {};
    static const QRegularExpression scriptPattern(
        "<script[^>]*id=[\"']lentille-context[\"'][^>]*>(.*?)</script>",
        QRegularExpression::DotMatchesEverythingOption |
            QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch scriptMatch = scriptPattern.match(html);
    if (scriptMatch.hasMatch()) {
        const QByteArray json = scriptMatch.captured(1).toUtf8();
        const QJsonDocument document = QJsonDocument::fromJson(json);
        if (document.isObject()) {
            const QJsonObject data = document.object().value("data").toObject();
            const QJsonObject info = data.value("problem").toObject();
            const QString pid = info.value("pid").toString();
            const QString name = info.value("name").toString();
            if (!pid.isEmpty() && !name.isEmpty()) {
                problem.pid = pid;
                problem.name = name;
                problem.difficulty = info.value("difficulty").toInt(0);
                problem.totalSubmit = info.value("totalSubmit").toInt(0);
                problem.totalAccepted = info.value("totalAccepted").toInt(0);
                problem.valid = true;
                return true;
            }
        }
    }

    static const QRegularExpression titlePattern(
        "<title>(.*?)</title>",
        QRegularExpression::DotMatchesEverythingOption |
            QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch titleMatch = titlePattern.match(html);
    if (titleMatch.hasMatch()) {
        QString title = titleMatch.captured(1).trimmed();
        if (!title.endsWith(" - 洛谷")) {
            error = "没有从洛谷页面中解析出题目信息。";
            return false;
        }
        title.chop(QString(" - 洛谷").size());
        title = title.trimmed();
        const int space = title.indexOf(' ');
        if (space > 0) {
            const QString pid = title.left(space).trimmed();
            const QString name = title.mid(space + 1).trimmed();
            if (!pid.isEmpty() && !name.isEmpty()) {
                problem.pid = pid;
                problem.name = name;
                problem.valid = true;
                return true;
            }
        }
    }

    error = "没有从洛谷页面中解析出题目信息。";
    return false;
}
