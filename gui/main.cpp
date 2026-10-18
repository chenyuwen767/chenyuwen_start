#include <QApplication>
#include <QCheckBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <string>
#include <vector>

#include "task_store.h"

class MainWindow : public QWidget {
public:
    MainWindow() : tasks_(loadTasks(dataFile_)) {
        setWindowTitle("算法练习任务本");
        resize(760, 460);

        auto* layout = new QVBoxLayout(this);
        summary_ = new QLabel(this);
        layout->addWidget(summary_);

        pendingOnly_ = new QCheckBox("只显示待完成题目", this);
        layout->addWidget(pendingOnly_);

        table_ = new QTableWidget(this);
        table_->setColumnCount(4);
        table_->setHorizontalHeaderLabels({"题目名称", "分类", "难度", "状态"});
        table_->setSelectionBehavior(QAbstractItemView::SelectRows);
        table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table_->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(table_);

        auto* buttons = new QHBoxLayout;
        auto* addButton = new QPushButton("新增题目", this);
        auto* completeButton = new QPushButton("标记为完成", this);
        buttons->addWidget(addButton);
        buttons->addWidget(completeButton);
        buttons->addStretch();
        layout->addLayout(buttons);

        connect(pendingOnly_, &QCheckBox::toggled, this, [this] { refresh(); });
        connect(addButton, &QPushButton::clicked, this, [this] { addTask(); });
        connect(completeButton, &QPushButton::clicked, this, [this] { completeTask(); });
        refresh();
    }

private:
    const std::string dataFile_ = "tasks.txt";
    std::vector<Task> tasks_;
    QLabel* summary_;
    QCheckBox* pendingOnly_;
    QTableWidget* table_;

    void refresh() {
        table_->setRowCount(0);
        int completed = 0;

        for (std::size_t index = 0; index < tasks_.size(); ++index) {
            const Task& task = tasks_[index];
            if (task.completed) {
                ++completed;
            }
            if (pendingOnly_->isChecked() && task.completed) {
                continue;
            }

            const int row = table_->rowCount();
            table_->insertRow(row);
            auto* title = new QTableWidgetItem(QString::fromUtf8(task.title.c_str()));
            title->setData(Qt::UserRole, static_cast<int>(index));
            table_->setItem(row, 0, title);
            table_->setItem(row, 1, new QTableWidgetItem(QString::fromUtf8(task.category.c_str())));
            table_->setItem(row, 2, new QTableWidgetItem(QString::fromUtf8(task.difficulty.c_str())));
            table_->setItem(row, 3, new QTableWidgetItem(task.completed ? "已完成" : "待完成"));
        }

        summary_->setText(QString("共 %1 道题，已完成 %2 道").arg(tasks_.size()).arg(completed));
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
        const int row = table_->currentRow();
        if (row < 0) {
            QMessageBox::information(this, "请选择题目", "请先在表格中选择一道题目。");
            return;
        }

        const int index = table_->item(row, 0)->data(Qt::UserRole).toInt();
        tasks_[index].completed = true;
        saveAndRefresh();
    }

    void saveAndRefresh() {
        if (!saveTasks(tasks_, dataFile_)) {
            QMessageBox::critical(this, "保存失败", "无法写入 tasks.txt。");
            return;
        }
        refresh();
    }
};

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    MainWindow window;
    window.show();
    return application.exec();
}
