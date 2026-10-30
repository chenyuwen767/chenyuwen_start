#include <QApplication>
#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QSet>
#include <QSettings>
#include <QShortcut>
#include <QStandardPaths>
#include <QStringList>
#include <QTableWidget>
#include <QTabWidget>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWidget>

#include <filesystem>
#include <string>
#include <vector>

#include "app_paths.h"
#include "task_store.h"

namespace {

QString taskText(const std::string& value) {
    return QString::fromUtf8(value.c_str());
}

std::string toTaskText(const QString& value) {
    return value.toUtf8().toStdString();
}

QString displayPath(const std::filesystem::path& path) {
    return QString::fromStdWString(path.wstring());
}

QString optionalNumber(const QJsonValue& value) {
    return value.isDouble() ? QString::number(value.toInt()) : QString("未公开");
}

bool promptTaskFields(QWidget* parent,
                      const QString& dialogTitle,
                      QString& title,
                      QString& category,
                      QString& difficulty) {
    QDialog dialog(parent);
    dialog.setWindowTitle(dialogTitle);

    auto* layout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout;
    auto* titleEdit = new QLineEdit(title, &dialog);
    auto* categoryEdit = new QLineEdit(category, &dialog);
    auto* difficultyEdit = new QLineEdit(difficulty, &dialog);
    form->addRow("题目名称：", titleEdit);
    form->addRow("分类：", categoryEdit);
    form->addRow("难度：", difficultyEdit);
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    titleEdit->setFocus();

    while (dialog.exec() == QDialog::Accepted) {
        title = titleEdit->text().trimmed();
        category = categoryEdit->text().trimmed();
        difficulty = difficultyEdit->text().trimmed();
        if (!title.isEmpty() && !category.isEmpty() && !difficulty.isEmpty()) {
            return true;
        }
        QMessageBox::warning(&dialog, "无法保存", "题目名称、分类和难度都不能为空。");
    }
    return false;
}

std::filesystem::path resolveDataFile() {
    QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (directory.isEmpty()) {
        directory = QCoreApplication::applicationDirPath();
    }
    return app_paths::resolveDataFile(
        QCoreApplication::arguments(),
        QCoreApplication::applicationDirPath(),
        QDir::currentPath(),
        directory);
}

} // namespace

class MainWindow : public QWidget {
public:
    MainWindow() : dataFile_(resolveDataFile()), network_(new QNetworkAccessManager(this)) {
        const LoadResult loaded = loadTasksChecked(dataFile_);
        tasks_ = loaded.tasks;
        dataLoadFailed_ = !loaded.ok;

        setWindowTitle("算法练习任务本");
        resize(960, 680);
        const QByteArray geometry = settings_.value("window/geometry").toByteArray();
        if (!geometry.isEmpty()) {
            restoreGeometry(geometry);
        }
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
            QPushButton#dangerButton {
                background: #c9463d;
            }
            QPushButton#dangerButton:hover {
                background: #a9362f;
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

        auto* menuBar = new QMenuBar(this);
        auto* fileMenu = menuBar->addMenu("文件");
        auto* openDataAction = fileMenu->addAction("打开数据目录");
        QObject::connect(openDataAction, &QAction::triggered, this, [this] {
            QDesktopServices::openUrl(QUrl::fromLocalFile(displayPath(dataFile_.parent_path())));
        });
        fileMenu->addSeparator();
        auto* quitAction = fileMenu->addAction("退出");
        QObject::connect(quitAction, &QAction::triggered, this, &QWidget::close);

        auto* helpMenu = menuBar->addMenu("帮助");
        auto* aboutAction = helpMenu->addAction("关于");
        QObject::connect(aboutAction, &QAction::triggered, this, [this] {
            QMessageBox::about(
                this,
                "关于算法练习任务本",
                QString("算法练习任务本 %1\n\n数据文件：%2\n\n"
                        "仅读取 Codeforces 与洛谷的公开数据，不保存或传输账号密码。")
                    .arg(QCoreApplication::applicationVersion(), displayPath(dataFile_)));
        });
        layout->setMenuBar(menuBar);

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

        codeforcesHandleInput_->setText(settings_.value("codeforces/handle").toString());
        luoguHandleInput_->setText(settings_.value("luogu/handle").toString());
        pendingOnly_->setChecked(settings_.value("tasks/pendingOnly", false).toBool());
        refreshTasks();

        if (!loaded.ok) {
            QTimer::singleShot(0, this, [this, message = QString::fromStdString(loaded.error)] {
                QMessageBox::critical(
                    this,
                    "数据读取失败",
                    QString("无法读取任务数据：%1\n\n路径：%2\n\n为避免覆盖现有数据，继续保存前会再次确认。")
                        .arg(message, displayPath(dataFile_)));
            });
        } else if (!loaded.warnings.empty()) {
            QStringList warnings;
            for (const std::string& warning : loaded.warnings) {
                warnings << QString::fromStdString(warning);
            }
            QTimer::singleShot(0, this, [this, text = warnings.join('\n')] {
                QMessageBox::warning(this, "部分数据未读取", text);
            });
        }
    }

protected:
    void closeEvent(QCloseEvent* event) override {
        settings_.setValue("window/geometry", saveGeometry());
        QWidget::closeEvent(event);
    }

private:
    const std::filesystem::path dataFile_;
    std::vector<Task> tasks_;
    QNetworkAccessManager* network_;
    QSettings settings_;
    bool dataLoadFailed_ = false;
    bool codeforcesLoading_ = false;
    bool luoguLoading_ = false;
    QLabel* taskSummary_ = nullptr;
    QCheckBox* pendingOnly_ = nullptr;
    QLineEdit* taskSearch_ = nullptr;
    QTableWidget* taskTable_ = nullptr;
    QLineEdit* codeforcesHandleInput_ = nullptr;
    QPushButton* codeforcesButton_ = nullptr;
    QLabel* codeforcesSummary_ = nullptr;
    QString codeforcesProfileSummary_;
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

        auto* searchCard = createCard(page);
        auto* searchLayout = new QHBoxLayout(searchCard);
        auto* searchLabel = new QLabel("搜索", searchCard);
        taskSearch_ = new QLineEdit(searchCard);
        taskSearch_->setClearButtonEnabled(true);
        taskSearch_->setPlaceholderText("题目名称、分类或难度");
        searchLayout->addWidget(searchLabel);
        searchLayout->addWidget(taskSearch_);
        layout->addWidget(searchCard);

        taskTable_ = new QTableWidget(page);
        taskTable_->setColumnCount(4);
        taskTable_->setHorizontalHeaderLabels({"题目名称", "分类", "难度", "状态"});
        taskTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        taskTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        taskTable_->setAlternatingRowColors(true);
        taskTable_->setSortingEnabled(true);
        taskTable_->sortByColumn(0, Qt::AscendingOrder);
        taskTable_->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(taskTable_);

        auto* buttons = new QHBoxLayout;
        auto* addButton = new QPushButton("新增题目", page);
        auto* editButton = new QPushButton("编辑题目", page);
        auto* completeButton = new QPushButton("标记为完成", page);
        auto* pendingButton = new QPushButton("标记为待完成", page);
        auto* deleteButton = new QPushButton("删除题目", page);
        deleteButton->setObjectName("dangerButton");
        buttons->addWidget(addButton);
        buttons->addWidget(editButton);
        buttons->addWidget(completeButton);
        buttons->addWidget(pendingButton);
        buttons->addWidget(deleteButton);
        buttons->addStretch();
        layout->addLayout(buttons);

        connect(pendingOnly_, &QCheckBox::toggled, this, [this](bool checked) {
            settings_.setValue("tasks/pendingOnly", checked);
            refreshTasks();
        });
        connect(taskSearch_, &QLineEdit::textChanged, this, [this] { refreshTasks(); });
        connect(addButton, &QPushButton::clicked, this, [this] { addTask(); });
        connect(editButton, &QPushButton::clicked, this, [this] { editTask(); });
        connect(completeButton, &QPushButton::clicked, this, [this] { completeTask(); });
        connect(pendingButton, &QPushButton::clicked, this, [this] { reopenTask(); });
        connect(deleteButton, &QPushButton::clicked, this, [this] { deleteTask(); });
        connect(taskTable_, &QTableWidget::doubleClicked, this, [this] { editTask(); });

        auto* newShortcut = new QShortcut(QKeySequence::New, this);
        connect(newShortcut, &QShortcut::activated, this, [this] { addTask(); });
        auto* findShortcut = new QShortcut(QKeySequence::Find, this);
        connect(findShortcut, &QShortcut::activated, this, [this] {
            taskSearch_->setFocus();
            taskSearch_->selectAll();
        });
        auto* deleteShortcut = new QShortcut(QKeySequence::Delete, taskTable_);
        deleteShortcut->setContext(Qt::WidgetShortcut);
        connect(deleteShortcut, &QShortcut::activated, this, [this] { deleteTask(); });
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
        const bool sortingEnabled = taskTable_->isSortingEnabled();
        taskTable_->setSortingEnabled(false);
        taskTable_->setRowCount(0);
        const QString keyword = taskSearch_ ? taskSearch_->text().trimmed() : QString();
        int completed = 0;
        int visible = 0;

        for (std::size_t index = 0; index < tasks_.size(); ++index) {
            const Task& task = tasks_[index];
            if (task.completed) {
                ++completed;
            }
            if (pendingOnly_->isChecked() && task.completed) {
                continue;
            }
            if (!keyword.isEmpty()) {
                const QString haystack =
                    taskText(task.title) + ' ' + taskText(task.category) + ' ' + taskText(task.difficulty);
                if (!haystack.contains(keyword, Qt::CaseInsensitive)) {
                    continue;
                }
            }

            ++visible;
            const int row = taskTable_->rowCount();
            taskTable_->insertRow(row);
            auto* title = new QTableWidgetItem(taskText(task.title));
            title->setData(Qt::UserRole, static_cast<int>(index));
            taskTable_->setItem(row, 0, title);
            taskTable_->setItem(row, 1, new QTableWidgetItem(taskText(task.category)));
            taskTable_->setItem(row, 2, new QTableWidgetItem(taskText(task.difficulty)));
            taskTable_->setItem(row, 3, new QTableWidgetItem(task.completed ? "已完成" : "待完成"));
        }

        QString summary = QString("共 %1 道题，已完成 %2 道").arg(tasks_.size()).arg(completed);
        if (visible != static_cast<int>(tasks_.size())) {
            summary += QString("，当前显示 %1 道").arg(visible);
        }
        taskSummary_->setText(summary);
        taskTable_->setSortingEnabled(sortingEnabled);
    }

    void addTask() {
        QString title;
        QString category;
        QString difficulty;
        if (!promptTaskFields(this, "新增题目", title, category, difficulty)) {
            return;
        }

        tasks_.push_back({toTaskText(title), toTaskText(category), toTaskText(difficulty), false});
        saveAndRefresh();
    }

    int selectedTaskIndex() const {
        const int row = taskTable_->currentRow();
        if (row < 0) {
            return -1;
        }
        const QTableWidgetItem* item = taskTable_->item(row, 0);
        return item ? item->data(Qt::UserRole).toInt() : -1;
    }

    void editTask() {
        const int index = selectedTaskIndex();
        if (index < 0) {
            QMessageBox::information(this, "请选择题目", "请先在表格中选择一道题目。");
            return;
        }

        const Task& task = tasks_[static_cast<std::size_t>(index)];
        QString title = taskText(task.title);
        QString category = taskText(task.category);
        QString difficulty = taskText(task.difficulty);
        if (!promptTaskFields(this, "编辑题目", title, category, difficulty)) {
            return;
        }

        tasks_[static_cast<std::size_t>(index)].title = toTaskText(title);
        tasks_[static_cast<std::size_t>(index)].category = toTaskText(category);
        tasks_[static_cast<std::size_t>(index)].difficulty = toTaskText(difficulty);
        saveAndRefresh();
    }

    void completeTask() {
        setTaskCompleted(true);
    }

    void reopenTask() {
        setTaskCompleted(false);
    }

    void setTaskCompleted(bool completed) {
        const int index = selectedTaskIndex();
        if (index < 0) {
            QMessageBox::information(this, "请选择题目", "请先在表格中选择一道题目。");
            return;
        }
        tasks_[static_cast<std::size_t>(index)].completed = completed;
        saveAndRefresh();
    }

    void deleteTask() {
        const int index = selectedTaskIndex();
        if (index < 0) {
            QMessageBox::information(this, "请选择题目", "请先在表格中选择一道题目。");
            return;
        }

        const QString title = taskText(tasks_[static_cast<std::size_t>(index)].title);
        const auto answer = QMessageBox::question(
            this,
            "删除题目",
            QString("确定删除“%1”吗？\n\n此操作不可撤销。").arg(title),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            return;
        }

        tasks_.erase(tasks_.begin() + index);
        saveAndRefresh();
    }

    void saveAndRefresh() {
        if (dataLoadFailed_) {
            const auto answer = QMessageBox::warning(
                this,
                "数据尚未成功读取",
                "启动时未能读取原任务数据。继续保存会用当前列表覆盖数据文件。\n\n是否继续？",
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
            if (answer != QMessageBox::Yes) {
                return;
            }
        }

        std::string error;
        if (!saveTasksChecked(tasks_, dataFile_, error)) {
            QMessageBox::critical(
                this,
                "保存失败",
                QString("无法写入任务数据：%1\n\n路径：%2")
                    .arg(QString::fromStdString(error), displayPath(dataFile_)));
            return;
        }

        dataLoadFailed_ = false;
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

    QNetworkRequest networkRequest(const QUrl& url) const {
        QNetworkRequest request(url);
        request.setTransferTimeout(15000);
        return request;
    }

    QNetworkRequest luoguRequest(const QUrl& url) const {
        QNetworkRequest request = networkRequest(url);
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
        if (codeforcesLoading_) {
            return;
        }

        const QString handle = codeforcesHandleInput_->text().trimmed();
        if (handle.isEmpty()) {
            QMessageBox::information(this, "需要账号", "请输入 Codeforces handle。");
            return;
        }

        codeforcesLoading_ = true;
        codeforcesButton_->setEnabled(false);
        settings_.setValue("codeforces/handle", handle);
        codeforcesSummary_->setText("正在查询账号资料……");
        codeforcesSubmissionsTable_->setRowCount(0);

        auto* reply = network_->get(networkRequest(codeforcesApiUrl("user.info", "handles", handle)));
        connect(reply, &QNetworkReply::finished, this, [this, reply, handle] {
            QJsonArray users;
            QString error;
            if (!readCodeforcesResult(reply, users, error) || users.isEmpty()) {
                reply->deleteLater();
                codeforcesLoading_ = false;
                codeforcesSummary_->setText("查询失败：" + error);
                codeforcesButton_->setEnabled(true);
                return;
            }

            const QJsonObject user = users.first().toObject();
            const QString rank = user.value("rank").toString("未定级");
            const QString rating = user.contains("rating") ? QString::number(user.value("rating").toInt()) : "暂无";
            const QString maxRating = user.contains("maxRating") ? QString::number(user.value("maxRating").toInt()) : "暂无";
            codeforcesProfileSummary_ = QString("%1（%2）  当前 rating：%3  最高 rating：%4")
                                            .arg(user.value("handle").toString(handle))
                                            .arg(rank)
                                            .arg(rating)
                                            .arg(maxRating);
            codeforcesSummary_->setText(codeforcesProfileSummary_ + "\n正在读取最近提交……");
            reply->deleteLater();

            QTimer::singleShot(2100, this, [this, handle] { loadRecentCodeforcesSubmissions(handle); });
        });
    }

    void loadRecentCodeforcesSubmissions(const QString& handle) {
        auto* reply = network_->get(networkRequest(codeforcesApiUrl("user.status", "handle", handle)));
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            QJsonArray submissions;
            QString error;
            if (!readCodeforcesResult(reply, submissions, error)) {
                reply->deleteLater();
                codeforcesLoading_ = false;
                codeforcesSummary_->setText(codeforcesProfileSummary_ + "\n提交记录读取失败：" + error);
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

            codeforcesSummary_->setText(
                codeforcesProfileSummary_ +
                QString("\n最近 1000 次提交中通过 %1 道不同题目；返回提交数：%2")
                    .arg(solved.size())
                    .arg(submissions.size()));
            showCodeforcesSubmissions(submissions);
            reply->deleteLater();
            codeforcesLoading_ = false;
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
        if (luoguLoading_) {
            return;
        }

        const QString account = luoguHandleInput_->text().trimmed();
        if (account.isEmpty()) {
            QMessageBox::information(this, "需要账号", "请输入洛谷用户名或 UID。");
            return;
        }

        luoguLoading_ = true;
        luoguButton_->setEnabled(false);
        settings_.setValue("luogu/handle", account);
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
                : "未公开";
            luoguSummary_->setText(QString("%1（UID：%2）\n通过 %3 题 · 提交 %4 题 · 等级分：%5")
                                       .arg(user.value("name").toString())
                                       .arg(user.value("uid").toInt())
                                       .arg(optionalNumber(user.value("passedProblemCount")))
                                       .arg(optionalNumber(user.value("submittedProblemCount")))
                                       .arg(elo));
            luoguDetails_->setText(QString("CCF 等级：%1  ·  XCPC 等级：%2  ·  关注：%3  ·  粉丝：%4\n%5")
                                      .arg(optionalNumber(user.value("ccfLevel")))
                                      .arg(optionalNumber(user.value("xcpcLevel")))
                                      .arg(optionalNumber(user.value("followingCount")))
                                      .arg(optionalNumber(user.value("followerCount")))
                                      .arg(user.value("slogan").toString("这个人很懒，什么都没写")));
            reply->deleteLater();
            luoguLoading_ = false;
            luoguButton_->setEnabled(true);
        });
    }

    void showLuoguError(const QString& error) {
        luoguLoading_ = false;
        luoguSummary_->setText("查询失败：" + error);
        luoguDetails_->setText("请检查用户名或 UID，并确认账号资料公开可见。");
        luoguButton_->setEnabled(true);
    }
};

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName("chenyuwen");
    QCoreApplication::setApplicationName("AlgorithmPracticeNotebook");
    QCoreApplication::setApplicationVersion("0.3.1");
    MainWindow window;
    window.show();
    return application.exec();
}
