#include "task_table_model.h"

#include <QStringList>

QString taskTagsText(const std::vector<std::string>& tags) {
    QStringList values;
    for (const std::string& tag : tags) {
        values << QString::fromUtf8(tag.c_str());
    }
    return values.join(", ");
}

TaskTableModel::TaskTableModel(QObject* parent) : QAbstractTableModel(parent) {}

void TaskTableModel::setTasks(std::vector<Task>* tasks) {
    beginResetModel();
    tasks_ = tasks;
    endResetModel();
}

void TaskTableModel::reload() {
    beginResetModel();
    endResetModel();
}

int TaskTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() || tasks_ == nullptr) {
        return 0;
    }
    return static_cast<int>(tasks_->size());
}

int TaskTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : ColumnCount;
}

QVariant TaskTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || tasks_ == nullptr || index.row() < 0 ||
        index.row() >= static_cast<int>(tasks_->size())) {
        return {};
    }

    const Task& task = (*tasks_)[static_cast<std::size_t>(index.row())];
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case TitleColumn:
            return QString::fromUtf8(task.title.c_str());
        case CategoryColumn:
            return QString::fromUtf8(task.category.c_str());
        case DifficultyColumn:
            return QString::fromUtf8(task.difficulty.c_str());
        case TagsColumn:
            return taskTagsText(task.tags);
        case StatusColumn:
            return task.completed ? QString("已完成") : QString("待完成");
        default:
            return {};
        }
    }

    if (role == Qt::ToolTipRole && index.column() == TitleColumn) {
        return QString::fromUtf8(task.notes.c_str());
    }
    if (role == Qt::TextAlignmentRole && index.column() == StatusColumn) {
        return static_cast<int>(Qt::AlignCenter);
    }
    return {};
}

QVariant TaskTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section) {
    case TitleColumn:
        return QString("题目名称");
    case CategoryColumn:
        return QString("分类");
    case DifficultyColumn:
        return QString("难度");
    case TagsColumn:
        return QString("标签");
    case StatusColumn:
        return QString("状态");
    default:
        return {};
    }
}

const Task* TaskTableModel::taskAt(int row) const {
    if (tasks_ == nullptr || row < 0 || row >= static_cast<int>(tasks_->size())) {
        return nullptr;
    }
    return &(*tasks_)[static_cast<std::size_t>(row)];
}

TaskFilterProxyModel::TaskFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {
    setDynamicSortFilter(true);
}

void TaskFilterProxyModel::setKeyword(const QString& keyword) {
    if (keyword_ == keyword) {
        return;
    }
    keyword_ = keyword;
    notifyFilterChanged();
}

void TaskFilterProxyModel::setPendingOnly(bool pendingOnly) {
    if (pendingOnly_ == pendingOnly) {
        return;
    }
    pendingOnly_ = pendingOnly;
    notifyFilterChanged();
}

void TaskFilterProxyModel::notifyFilterChanged() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 12, 0)
    beginFilterChange();
    endFilterChange();
#else
    invalidateFilter();
#endif
}

bool TaskFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    if (sourceParent.isValid()) {
        return false;
    }

    const auto* model = static_cast<const TaskTableModel*>(sourceModel());
    const Task* task = model ? model->taskAt(sourceRow) : nullptr;
    if (task == nullptr) {
        return false;
    }
    if (pendingOnly_ && task->completed) {
        return false;
    }
    if (keyword_.isEmpty()) {
        return true;
    }

    const QString haystack =
        QString::fromUtf8(task->title.c_str()) + ' ' +
        QString::fromUtf8(task->category.c_str()) + ' ' +
        QString::fromUtf8(task->difficulty.c_str()) + ' ' +
        taskTagsText(task->tags) + ' ' +
        QString::fromUtf8(task->notes.c_str());
    return haystack.contains(keyword_, Qt::CaseInsensitive);
}
