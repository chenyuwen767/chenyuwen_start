#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QFrame>
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
        resize(960, 680);
        setStyleSheet(R"(
            QWidget {
                background: #f5f7fb;
                color: #24324a;
                font-family: "Microsoft YaHei";
                font-size: 14px;
            }
            QLabel#appTitle {
                color: #173b72;
                font-size: 25px;
                font-weight: 700;
            }
            QLabel#pageTitle {
                color: #173b72;
                font-size: 20px;
                font-weight: 700;
            }
            QLabel#pageDescription, QLabel#hint {
                color: #6a7890;
            }
            QFrame#card {
                background: white;
                border: 1px solid #e1e7f0;
                border-radius: 10px;
            }
            QLabel#summaryCard {
                background: #eef5ff;
                border: 1px solid #cfe0fa;
                border-radius: 8px;
                color: #1c467f;
                font-size: 15px;
                font-weight: 600;
                padding: 12px;
            }
            QLineEdit {
                background: white;
                border: 1px solid #cfd8e6;
                border-radius: 7px;
                padding: 8px 10px;
                selection-background-color: #2c78d4;
            }
            QLineEdit:focus {
                border: 2px solid #4d91e7;
            }
            QPushButton {
                background: #2f7fd8;
                border: 0;
                border-radius: 7px;
                color: white;
                font-weight: 600;
                padding: 8px 16px;
            }
            QPushButton:hover {
                background: #2169b8;
            }
            QPushButton:disabled {
                background: #aab8c9;
            }
            QTabWidget::pane {
                background: white;
                border: 1px solid #dfe6f0;
                border-radius: 10px;
                top: -1px;
            }
            QTabBar::tab {
                background: #e9eef6;
                border: 0;
                border-top-left-radius: 8px;
                border-top-right-radius: 8px;
                color: #52637c;
                margin-right: 4px;
                padding: 10px 20px;
            }
            QTabBar::tab:selected {
                background: white;
                color: #1d5fae;
                font-weight: 700;
            }
            QHeaderView::section {
                background: #edf3fb;
                border: 0;
                border-bottom: 1px solid #dbe5f1;
                color: #42516a;
                font-weight: 700;
                padding: 9px;
            }
            QTableWidget {
                alternate-background-color: #f8faff;
                background: white;
                border: 1px solid #dfe6f0;
                border-radius: 8px;
                gridline-color: #edf1f6;
                selection-background-color: #d9eafe;
                selection-color: #1b3d69;
            }
            QCheckBox {
                color: #52637c;
                spacing: 7px;
            }
        )");

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(22, 18, 22, 22);
        layout->setSpacing(14);

        auto* title = new QLabel("算法练习任务本", this);
        title->setObjectName("appTitle");
        auto* subtitle = new QLabel("集中记录练习任务，并查看在线评测平台的公开进度", this);
        subtitle->setObjectName("pageDescription");
        layout->addWidget(title);
        layout->addWidget(subtitle);

        auto* tabs = new QTabWidget(this);
        tabs->addTab(createTasksPage(), "练习任务");
        tabs->addTab(createCodeforcesPage(), "Codeforces");
        tabs->addTab(createLuoguPage(), "洛谷");
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
    QLineEdit* codeforcesHandleInput_ = nullptr;
    QPushButton* codeforcesButton_ = nullptr;
    QLabel* codeforcesSummary_ = nullptr;
    QTableWidget* codeforcesSubmissionsTable_ = nullptr;
    QLineEdit* luoguHandleInput_ = nullptr;
    QPushButton* luoguButton_ = nullptr;
    QLabel* luoguSummary_ = nullptr;
    QLabel* luoguDetails_ = nullptr;

    void addPageHeading(QVBoxLayout* layout, const QString& title, const QString& description) {
        auto* heading = new QLabel(title);
        heading->setObjectName("pageTitle");
        auto* note = new QLabel(description);
        note->setObjectName("pageDescription");
        note->setWordWrap(true);
        layout->addWidget(heading);
        layout->addWidget(note);
    }

    QFrame* createCard(QWidget* parent) {
        auto* card = new QFrame(parent);
        card->setObjectName("card");
        return card;
    }

    QWidget* createTasksPage() {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(12);
        addPageHeading(layout, "我的练习任务", "把算法题目整理成可持续推进的练习清单。");

        auto* card = createCard(page);
        auto* cardLayout = new QVBoxLayout(card);
        taskSummary_ = new QLabel(card);
        taskSummary_->setObjectName("summaryCard");
        pendingOnly_ = new QCheckBox("只显示待完成题目", card);
        cardLayout->addWidget(taskSummary_);
        cardLayout->addWidget(pendingOnly_);
        layout->addWidget(card);

        taskTable_ = new QTableWidget(page);
        taskTable_->setColumnCount(4);
        taskTable_->setHorizontalHeaderLabels({"题目名称", "分类", "难度", "状态"});
        taskTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        taskTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        taskTable_->setAlternatingRowColors(true);
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
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(12);
        addPageHeading(layout, "Codeforces 进度", "查询公开账号资料和最近提交记录。");

        auto* searchCard = createCard(page);
        auto* searchLayout = new QHBoxLayout(searchCard);
        codeforcesHandleInput_ = new QLineEdit(searchCard);
        codeforcesHandleInput_->setPlaceholderText("输入 Codeforces handle");
        codeforcesButton_ = new QPushButton("查询账号", searchCard);
        searchLayout->addWidget(new QLabel("账号", searchCard));
        searchLayout->addWidget(codeforcesHandleInput_);
        searchLayout->addWidget(codeforcesButton_);
        layout->addWidget(searchCard);

        codeforcesSummary_ = new QLabel("输入公开的 Codeforces handle 后查询个人资料和提交记录。", page);
        codeforcesSummary_->setObjectName("summaryCard");
        codeforcesSummary_->setWordWrap(true);
        layout->addWidget(codeforcesSummary_);

        auto* note = new QLabel("通过题数根据最近 1000 次提交统计；完整历史可能超出此范围。", page);
        note->setObjectName("hint");
        note->setWordWrap(true);
        layout->addWidget(note);

        codeforcesSubmissionsTable_ = new QTableWidget(page);
        codeforcesSubmissionsTable_->setColumnCount(4);
        codeforcesSubmissionsTable_->setHorizontalHeaderLabels({"提交时间", "题目", "结果", "语言"});
        codeforcesSubmissionsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        codeforcesSubmissionsTable_->setAlternatingRowColors(true);
        codeforcesSubmissionsTable_->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(codeforcesSubmissionsTable_);

        connect(codeforcesButton_, &QPushButton::clicked, this, [this] { loadCodeforcesAccount(); });
        connect(codeforcesHandleInput_, &QLineEdit::returnPressed, this, [this] { loadCodeforcesAccount(); });
        return page;
    }

    QWidget* createLuoguPage() {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(12);
        addPageHeading(layout, "洛谷账号信息", "输入用户名或 UID，查询公开账号资料和解题统计。");

        auto* searchCard = createCard(page);
        auto* searchLayout = new QHBoxLayout(searchCard);
        luoguHandleInput_ = new QLineEdit(searchCard);
        luoguHandleInput_->setPlaceholderText("输入洛谷用户名或 UID");
        luoguButton_ = new QPushButton("查询账号", searchCard);
        searchLayout->addWidget(new QLabel("账号", searchCard));
        searchLayout->addWidget(luoguHandleInput_);
        searchLayout->addWidget(luoguButton_);
        layout->addWidget(searchCard);

        luoguSummary_ = new QLabel("输入公开的洛谷用户名或 UID 后查询资料。", page);
        luoguSummary_->setObjectName("summaryCard");
        luoguSummary_->setWordWrap(true);
        layout->addWidget(luoguSummary_);

        luoguDetails_ = new QLabel("可显示通过题数、提交题数、等级分和关注数据。", page);
        luoguDetails_->setObjectName("hint");
        luoguDetails_->setWordWrap(true);
        layout->addWidget(luoguDetails_);

        auto* limitationCard = createCard(page);
        auto* limitationLayout = new QVBoxLayout(limitationCard);
        auto* limitationTitle = new QLabel("提交记录说明", limitationCard);
        limitationTitle->setStyleSheet("font-weight: 700; color: #314d75;");
        auto* limitation = new QLabel(
            "洛谷当前可能要求登录后才能读取他人的提交明细。此页面只读取公开资料，不会请求、保存或传输账号密码。",
            limitationCard);
        limitation->setObjectName("hint");
        limitation->setWordWrap(true);
        limitationLayout->addWidget(limitationTitle);
        limitationLayout->addWidget(limitation);
        layout->addWidget(limitationCard);
        layout->addStretch();

        connect(luoguButton_, &QPushButton::clicked, this, [this] { loadLuoguAccount(); });
        connect(luoguHandleInput_, &QLineEdit::returnPressed, this, [this] { loadLuoguAccount(); });
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

    QUrl codeforcesApiUrl(const QString& method, const QString& parameter, const QString& value) const {
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

    QNetworkRequest luoguRequest(const QUrl& url) const {
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, "AlgorithmPracticeNotebook/1.0");
        return request;
    }

    bool readCodeforcesResult(QNetworkReply* reply, QJsonArray& result, QString& error) const {
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

    bool readJsonObject(QNetworkReply* reply, QJsonObject& result, QString& error) const {
        if (reply->error() != QNetworkReply::NoError) {
            error = reply->errorString();
            return false;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            error = "服务返回了无法识别的数据。";
            return false;
        }

        result = document.object();
        return true;
    }

    void loadCodeforcesAccount() {
        const QString handle = codeforcesHandleInput_->text().trimmed();
        if (handle.isEmpty()) {
            QMessageBox::information(this, "需要账号", "请输入 Codeforces handle。");
            return;
        }

        codeforcesButton_->setEnabled(false);
        codeforcesSummary_->setText("正在查询账号资料……");
        codeforcesSubmissionsTable_->setRowCount(0);

        auto* reply = network_->get(QNetworkRequest(codeforcesApiUrl("user.info", "handles", handle)));
        connect(reply, &QNetworkReply::finished, this, [this, reply, handle] {
            QJsonArray users;
            QString error;
            if (!readCodeforcesResult(reply, users, error) || users.isEmpty()) {
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

            QTimer::singleShot(2100, this, [this, handle] { loadRecentCodeforcesSubmissions(handle); });
        });
    }

    void loadRecentCodeforcesSubmissions(const QString& handle) {
        auto* reply = network_->get(QNetworkRequest(codeforcesApiUrl("user.status", "handle", handle)));
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            QJsonArray submissions;
            QString error;
            if (!readCodeforcesResult(reply, submissions, error)) {
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
            showCodeforcesSubmissions(submissions);
            reply->deleteLater();
            codeforcesButton_->setEnabled(true);
        });
    }

    void showCodeforcesSubmissions(const QJsonArray& submissions) {
        codeforcesSubmissionsTable_->setRowCount(0);
        const int first = qMax(0, submissions.size() - 20);
        for (int index = submissions.size() - 1; index >= first; --index) {
            const QJsonObject submission = submissions.at(index).toObject();
            const QJsonObject problem = submission.value("problem").toObject();
            const int row = codeforcesSubmissionsTable_->rowCount();
            codeforcesSubmissionsTable_->insertRow(row);

            const qint64 timestamp = submission.value("creationTimeSeconds").toVariant().toLongLong();
            const QString time = QDateTime::fromSecsSinceEpoch(timestamp).toString("yyyy-MM-dd HH:mm");
            const QString problemName = QString("%1 %2")
                                            .arg(problem.value("index").toString(),
                                                 problem.value("name").toString());
            const QString verdict = submission.value("verdict").toString("等待评测");
            const QString language = submission.value("programmingLanguage").toString();
            codeforcesSubmissionsTable_->setItem(row, 0, new QTableWidgetItem(time));
            codeforcesSubmissionsTable_->setItem(row, 1, new QTableWidgetItem(problemName));
            codeforcesSubmissionsTable_->setItem(row, 2, new QTableWidgetItem(verdict));
            codeforcesSubmissionsTable_->setItem(row, 3, new QTableWidgetItem(language));
        }
    }

    void loadLuoguAccount() {
        const QString account = luoguHandleInput_->text().trimmed();
        if (account.isEmpty()) {
            QMessageBox::information(this, "需要账号", "请输入洛谷用户名或 UID。");
            return;
        }

        luoguButton_->setEnabled(false);
        luoguSummary_->setText("正在查询公开资料……");
        luoguDetails_->setText("正在读取账号统计……");

        bool isUid = false;
        const int uid = account.toInt(&isUid);
        if (isUid) {
            loadLuoguProfile(uid);
            return;
        }

        QUrl url("https://www.luogu.com.cn/api/user/search");
        QUrlQuery query;
        query.addQueryItem("keyword", account);
        url.setQuery(query);

        auto* reply = network_->get(luoguRequest(url));
        connect(reply, &QNetworkReply::finished, this, [this, reply, account] {
            QJsonObject response;
            QString error;
            if (!readJsonObject(reply, response, error)) {
                reply->deleteLater();
                showLuoguError(error);
                return;
            }

            const QJsonArray users = response.value("users").toArray();
            if (users.isEmpty()) {
                reply->deleteLater();
                showLuoguError("未找到匹配的公开账号。");
                return;
            }

            QJsonObject selected = users.first().toObject();
            for (const QJsonValue& value : users) {
                const QJsonObject candidate = value.toObject();
                if (candidate.value("name").toString().compare(account, Qt::CaseInsensitive) == 0) {
                    selected = candidate;
                    break;
                }
            }

            reply->deleteLater();
            loadLuoguProfile(selected.value("uid").toInt());
        });
    }

    void loadLuoguProfile(int uid) {
        const QUrl url("https://www.luogu.com.cn/api/user/info/" + QString::number(uid));
        auto* reply = network_->get(luoguRequest(url));
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            QJsonObject response;
            QString error;
            if (!readJsonObject(reply, response, error)) {
                reply->deleteLater();
                showLuoguError(error);
                return;
            }

            const QJsonObject user = response.value("user").toObject();
            if (user.isEmpty()) {
                reply->deleteLater();
                showLuoguError("该账号资料不可见或不存在。");
                return;
            }

            const QString elo = user.value("eloValue").isDouble()
                ? QString::number(user.value("eloValue").toDouble(), 'f', 0)
                : "暂无";
            luoguSummary_->setText(QString("%1（UID：%2）\n通过 %3 题 · 提交 %4 题 · 等级分：%5")
                                       .arg(user.value("name").toString())
                                       .arg(user.value("uid").toInt())
                                       .arg(user.value("passedProblemCount").toInt())
                                       .arg(user.value("submittedProblemCount").toInt())
                                       .arg(elo));
            luoguDetails_->setText(QString("CCF 等级：%1  ·  XCPC 等级：%2  ·  关注：%3  ·  粉丝：%4\n%5")
                                      .arg(user.value("ccfLevel").toInt())
                                      .arg(user.value("xcpcLevel").toInt())
                                      .arg(user.value("followingCount").toInt())
                                      .arg(user.value("followerCount").toInt())
                                      .arg(user.value("slogan").toString("这个人很懒，什么都没写")));
            reply->deleteLater();
            luoguButton_->setEnabled(true);
        });
    }

    void showLuoguError(const QString& error) {
        luoguSummary_->setText("查询失败：" + error);
        luoguDetails_->setText("请检查用户名或 UID，并确认账号资料公开可见。");
        luoguButton_->setEnabled(true);
    }
};

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    MainWindow window;
    window.show();
    return application.exec();
}
