#include <QtTest>

#include <QCoreApplication>

#include "codeforces_problem.h"

class CodeforcesProblemTest : public QObject {
    Q_OBJECT

private slots:
    void normalizesProblemIds();
    void rejectsNonCodeforcesIds();
    void buildsProblemKeys();
    void rejectsInvalidProblemKeys();
    void buildsProblemUrls();
};

void CodeforcesProblemTest::normalizesProblemIds() {
    QCOMPARE(normalizeCodeforcesProblemId("1900A"), QString("1900A"));
    QCOMPARE(normalizeCodeforcesProblemId("CF 1900A"), QString("1900A"));
    QCOMPARE(normalizeCodeforcesProblemId("cf-1900-a"), QString("1900A"));
    QCOMPARE(normalizeCodeforcesProblemId(" 1900 a "), QString("1900A"));
    QCOMPARE(normalizeCodeforcesProblemId("1900A1"), QString("1900A1"));
}

void CodeforcesProblemTest::rejectsNonCodeforcesIds() {
    QVERIFY(normalizeCodeforcesProblemId("P1001").isEmpty());
    QVERIFY(normalizeCodeforcesProblemId("ABC").isEmpty());
    QVERIFY(normalizeCodeforcesProblemId("1900").isEmpty());
    QVERIFY(normalizeCodeforcesProblemId("").isEmpty());
}

void CodeforcesProblemTest::buildsProblemKeys() {
    QCOMPARE(codeforcesProblemKey(1900, "A"), QString("1900A"));
    QCOMPARE(codeforcesProblemKey(1900, "a"), QString("1900A"));
    QCOMPARE(codeforcesProblemKey(2261, "F1"), QString("2261F1"));
}

void CodeforcesProblemTest::rejectsInvalidProblemKeys() {
    QVERIFY(codeforcesProblemKey(0, "A").isEmpty());
    QVERIFY(codeforcesProblemKey(1900, "").isEmpty());
    QVERIFY(codeforcesProblemKey(1900, "1A").isEmpty());
    QVERIFY(codeforcesProblemKey(-1, "A").isEmpty());
}

void CodeforcesProblemTest::buildsProblemUrls() {
    QCOMPARE(codeforcesProblemUrl("1900A").toString(),
             QString("https://codeforces.com/problemset/problem/1900/A"));
    QCOMPARE(codeforcesProblemUrl("CF-1900-a").toString(),
             QString("https://codeforces.com/problemset/problem/1900/A"));
    QVERIFY(codeforcesProblemUrl("not-a-problem").isEmpty());
}

int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);
    CodeforcesProblemTest instance;
    return QTest::qExec(&instance, argc, argv);
}

#include "test_codeforces_problem.moc"
