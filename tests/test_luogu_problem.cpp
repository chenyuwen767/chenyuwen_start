#include <QtTest>

#include <QCoreApplication>

#include "luogu_problem.h"

class LuoguProblemTest : public QObject {
    Q_OBJECT

private slots:
    void normalizesProblemIdsAndUrls();
    void parsesEmbeddedProblemJson();
    void fallsBackToPageTitle();
    void rejectsNonProblemPages();
    void mapsDifficultyLabels();
};

void LuoguProblemTest::normalizesProblemIdsAndUrls() {
    QCOMPARE(normalizeLuoguProblemId("P1001"), QString("P1001"));
    QCOMPARE(normalizeLuoguProblemId("  p1001  "), QString("p1001"));
    QCOMPARE(normalizeLuoguProblemId("https://www.luogu.com.cn/problem/P1001?foo=1"),
             QString("P1001"));
    QVERIFY(normalizeLuoguProblemId("P 1001").isEmpty());
    QVERIFY(normalizeLuoguProblemId("").isEmpty());
    QCOMPARE(luoguProblemUrl("P1001").toString(),
             QString("https://www.luogu.com.cn/problem/P1001"));
}

void LuoguProblemTest::parsesEmbeddedProblemJson() {
    const QString html = R"(
<html><head><title>P1001 A+B Problem - 洛谷</title></head><body>
<script id="lentille-context" type="application/json">{"data":{"problem":{"pid":"P1001","name":"A+B Problem","difficulty":1,"totalSubmit":100,"totalAccepted":60}}}</script>
</body></html>)";

    LuoguProblem problem;
    QString error;
    QVERIFY(parseLuoguProblemHtml(html, problem, error));
    QVERIFY(error.isEmpty());
    QCOMPARE(problem.pid, QString("P1001"));
    QCOMPARE(problem.name, QString("A+B Problem"));
    QCOMPARE(problem.difficulty, 1);
    QCOMPARE(problem.totalSubmit, 100);
    QCOMPARE(problem.totalAccepted, 60);
    QVERIFY(problem.valid);
}

void LuoguProblemTest::fallsBackToPageTitle() {
    const QString html = "<html><head><title>B2001 入门测试 - 洛谷</title></head></html>";
    LuoguProblem problem;
    QString error;
    QVERIFY(parseLuoguProblemHtml(html, problem, error));
    QCOMPARE(problem.pid, QString("B2001"));
    QCOMPARE(problem.name, QString("入门测试"));
}

void LuoguProblemTest::rejectsNonProblemPages() {
    LuoguProblem problem;
    QString error;
    QVERIFY(!parseLuoguProblemHtml("<html><head><title>Just a moment...</title></head></html>",
                                   problem,
                                   error));
    QVERIFY(!error.isEmpty());
}

void LuoguProblemTest::mapsDifficultyLabels() {
    QCOMPARE(luoguDifficultyLabel(0), QString("暂无评定"));
    QCOMPARE(luoguDifficultyLabel(1), QString("入门"));
    QCOMPARE(luoguDifficultyLabel(7), QString("NOI/NOI+/CTSC"));
    QCOMPARE(luoguDifficultyLabel(99), QString("未评定"));
}

int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);
    LuoguProblemTest instance;
    return QTest::qExec(&instance, argc, argv);
}

#include "test_luogu_problem.moc"
