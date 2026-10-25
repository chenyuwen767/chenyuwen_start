#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QSet>
#include <QTableWidget>
#include <QTabWidget>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWidget>

#include <string>
#include <vector>

#include "task_store.h"

class MainWindow : public QWidget {
public:
    MainWindow() : tasks_(loadTasks(dataFile_)), network_(new QNetworkAccessManager(this)) {
        setWindowTitle("算法练习任务本");
        resize(850, 560);

        auto* layout = new QVBoxLayout(this);
        auto* tabs = new QTabWidget(this);
        tabs->addTab(createTasksPage(), "练习任务");
        tabs->addTab(createCodeforcesPage(), "Codeforces");
        layout->addWidget(tabs);
        refreshTasks();
    }

private:
    const std::string dataFile_ = "tasks.txt";
    std::vector<Task> tasks_;
    QNetworkAccessManager* network_;
    QLabel* taskSummary_ = nullptr;
    QCheckBox* pendingOnly_ = nullptr;
    QTableWidget* taskTable_ = nullptr;
    QLineEdit* handleInput_ = nullptr;
    QPushButton* codeforcesButton_ = nullptr;
    QLabel* codeforcesSummary_ = nullptr;
    QTableWidget* submissionsTable_ = nullptr;

    QWidget* createTasksPage() {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        taskSummary_ = new QLabel(page);
        layout->addWidget(taskSummary_);

        pendingOnly_ = new QCheckBox("只显示待完成题目", page);
        layout->addWidget(pendingOnly_);

        taskTable_ = new QTableWidget(page);
        taskTable_->setColumnCount(4);
        taskTable_->setHorizontalHeaderLabels({"题目名称", "分类", "难度", "状态"});
        taskTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        taskTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        taskTable_->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(taskTable_);

        auto* buttons = new QHBoxLayout;
        auto* addButton = new QPushButton("新增题目", page);
        auto* completeButton = new QPushButton("标记为完成", page);
        buttons->addWidget(addButton);
        buttons->addWidget(completeButton);
        buttons->addStretch();
        layout->addLayout(buttons);

        connect(pendingOnly_, &QCheckBox::toggled, this, [this] { refreshTasks(); });
        connect(addButton, &QPushButton::clicked, this, [this] { addTask(); });
        connect(completeButton, &QPushButton::clicked, this, [this] { completeTask(); });
        return page;
    }

    QWidget* createCodeforcesPage() {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto* accountRow = new QHBoxLayout;
        handleInput_ = new QLineEdit(page);
        handleInput_->setPlaceholderText("输入 Codeforces handle");
        codeforcesButton_ = new QPushButton("查询", page);
        accountRow->addWidget(new QLabel("账号：", page));
        accountRow->addWidget(handleInput_);
        accountRow->addWidget(codeforcesButton_);
        layout->addLayout(accountRow);

        codeforcesSummary_ = new QLabel("输入公开的 Codeforces handle 后查询个人资料和提交记录。", page);
        codeforcesSummary_->setWordWrap(true);
        layout->addWidget(codeforcesSummary_);

        auto* note = new QLabel("通过题数根据最近 1000 次提交统计；完整历史可能超出此范围。查询公开数据无需密码。", page);
        note->setWordWrap(true);
        layout->addWidget(note);

        submissionsTable_ = new QTableWidget(page);
        submissionsTable_->setColumnCount(4);
        submissionsTable_->setHorizontalHeaderLabels({"提交时间", "题目", "结果", "语言"});
        submissionsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        submissionsTable_->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(submissionsTable_);

        connect(codeforcesButton_, &QPushButton::clicked, this, [this] { loadCodeforcesAccount(); });
        connect(handleInput_, &QLineEdit::returnPressed, this, [this] { loadCodeforcesAccount(); });
        return page;
    }

    void refreshTasks() {
        taskTable_->setRowCount(0);
        int completed = 0;

        for (std::size_t index = 0; index < tasks_.size(); ++index) {
            const Task& task = tasks_[index];
            if (task.completed) {
                ++completed;
            }
            if (pendingOnly_->isChecked() && task.completed) {
                continue;
            }

            const int row = taskTable_->rowCount();
            taskTable_->insertRow(row);
            auto* title = new QTableWidgetItem(QString::fromUtf8(task.title.c_str()));
            title->setData(Qt::UserRole, static_cast<int>(index));
            taskTable_->setItem(row, 0, title);
            taskTable_->setItem(row, 1, new QTableWidgetItem(QString::fromUtf8(task.category.c_str())));
            taskTable_->setItem(row, 2, new QTableWidgetItem(QString::fromUtf8(task.difficulty.c_str())));
            taskTable_->setItem(row, 3, new QTableWidgetItem(task.completed ? "已完成" : "待完成"));
        }

        taskSummary_->setText(QString("共 %1 道题，已完成 %2 道").arg(tasks_.size()).arg(completed));
    }

    void addTask() {
        bool accepted = false;
        const QString title = QInputDialog::getText(this, "新增题目", "题目名称：", QLineEdit::Normal, {}, &accepted);
        if (!accepted) {
            return;
        }
        const QString category = QInputDialog::getText(this, "新增题目", "分类：", QLineEdit::Normal, {}, &accepted);
        if (!accepted) {
            return;
        }
        const QString difficulty = QInputDialog::getText(this, "新增题目", "难度：", QLineEdit::Normal, {}, &accepted);
        if (!accepted) {
            return;
        }
        if (title.trimmed().isEmpty() || category.trimmed().isEmpty() || difficulty.trimmed().isEmpty()) {
            QMessageBox::warning(this, "无法新增", "题目名称、分类和难度都不能为空。");
            return;
        }

        tasks_.push_back({title.toUtf8().toStdString(), category.toUtf8().toStdString(), difficulty.toUtf8().toStdString(), false});
        saveAndRefresh();
    }

    void completeTask() {
        const int row = taskTable_->currentRow();
        if (row < 0) {
            QMessageBox::information(this, "请选择题目", "请先在表格中选择一道题目。");
            return;
        }

        const int index = taskTable_->item(row, 0)->data(Qt::UserRole).toInt();
        tasks_[index].completed = true;
        saveAndRefresh();
    }

    void saveAndRefresh() {
        if (!saveTasks(tasks_, dataFile_)) {
            QMessageBox::critical(this, "保存失败", "无法写入 tasks.txt。");
            return;
        }
        refreshTasks();
    }

    QUrl apiUrl(const QString& method, const QString& parameter, const QString& value) const {
        QUrl url("https://codeforces.com/api/" + method);
        QUrlQuery query;
        query.addQueryItem(parameter, value);
        if (method == "user.status") {
            query.addQueryItem("from", "1");
            query.addQueryItem("count", "1000");
        }
        url.setQuery(query);
        return url;
    }

    bool readApiResult(QNetworkReply* reply, QJsonArray& result, QString& error) const {
        if (reply->error() != QNetworkReply::NoError) {
            error = reply->errorString();
            return false;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            error = "Codeforces 返回了无法识别的数据。";
            return false;
        }

        const QJsonObject object = document.object();
        if (object.value("status").toString() != "OK") {
            error = object.value("comment").toString("Codeforces API 请求失败。");
            return false;
        }

        result = object.value("result").toArray();
        return true;
    }

    void loadCodeforcesAccount() {
        const QString handle = handleInput_->text().trimmed();
        if (handle.isEmpty()) {
            QMessageBox::information(this, "需要账号", "请输入 Codeforces handle。");
            return;
        }

        codeforcesButton_->setEnabled(false);
        codeforcesSummary_->setText("正在查询账号资料……");
        submissionsTable_->setRowCount(0);

        auto* reply = network_->get(QNetworkRequest(apiUrl("user.info", "handles", handle)));
        connect(reply, &QNetworkReply::finished, this, [this, reply, handle] {
            QJsonArray users;
            QString error;
            if (!readApiResult(reply, users, error) || users.isEmpty()) {
                reply->deleteLater();
                codeforcesSummary_->setText("查询失败：" + error);
                codeforcesButton_->setEnabled(true);
                return;
            }

            const QJsonObject user = users.first().toObject();
            const QString rank = user.value("rank").toString("未定级");
            const QString rating = user.contains("rating") ? QString::number(user.value("rating").toInt()) : "暂无";
            const QString maxRating = user.contains("maxRating") ? QString::number(user.value("maxRating").toInt()) : "暂无";
            codeforcesSummary_->setText(QString("%1（%2）  当前 rating：%3  最高 rating：%4\n正在读取最近提交……")
                                            .arg(user.value("handle").toString(handle))
                                            .arg(rank)
                                            .arg(rating)
                                            .arg(maxRating));
            reply->deleteLater();

            QTimer::singleShot(2100, this, [this, handle] { loadRecentSubmissions(handle); });
        });
    }

    void loadRecentSubmissions(const QString& handle) {
        auto* reply = network_->get(QNetworkRequest(apiUrl("user.status", "handle", handle)));
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            QJsonArray submissions;
            QString error;
            if (!readApiResult(reply, submissions, error)) {
                reply->deleteLater();
                codeforcesSummary_->setText(codeforcesSummary_->text() + "\n提交记录读取失败：" + error);
                codeforcesButton_->setEnabled(true);
                return;
            }

            QSet<QString> solved;
            for (const QJsonValue& value : submissions) {
                const QJsonObject submission = value.toObject();
                if (submission.value("verdict").toString() != "OK") {
                    continue;
                }
                const QJsonObject problem = submission.value("problem").toObject();
                solved.insert(QString::number(problem.value("contestId").toInt()) + "/" +
                              problem.value("index").toString());
            }

            codeforcesSummary_->setText(codeforcesSummary_->text().replace(
                "正在读取最近提交……",
                QString("最近 1000 次提交中通过 %1 道不同题目；返回提交数：%2").arg(solved.size()).arg(submissions.size())));
            showSubmissions(submissions);
            reply->deleteLater();
            codeforcesButton_->setEnabled(true);
        });
    }

    void showSubmissions(const QJsonArray& submissions) {
        submissionsTable_->setRowCount(0);
        const int first = qMax(0, submissions.size() - 20);
        for (int index = submissions.size() - 1; index >= first; --index) {
            const QJsonObject submission = submissions.at(index).toObject();
            const QJsonObject problem = submission.value("problem").toObject();
            const int row = submissionsTable_->rowCount();
            submissionsTable_->insertRow(row);

            const qint64 timestamp = submission.value("creationTimeSeconds").toVariant().toLongLong();
            const QString time = QDateTime::fromSecsSinceEpoch(timestamp).toString("yyyy-MM-dd HH:mm");
            const QString problemName = QString("%1 %2")
                                            .arg(problem.value("index").toString(),
                                                 problem.value("name").toString());
            const QString verdict = submission.value("verdict").toString("等待评测");
            const QString language = submission.value("programmingLanguage").toString();
            submissionsTable_->setItem(row, 0, new QTableWidgetItem(time));
            submissionsTable_->setItem(row, 1, new QTableWidgetItem(problemName));
            submissionsTable_->setItem(row, 2, new QTableWidgetItem(verdict));
            submissionsTable_->setItem(row, 3, new QTableWidgetItem(language));
        }
    }
};

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    MainWindow window;
    window.show();
    return application.exec();
}
