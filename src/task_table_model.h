#pragma once

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QString>

#include <vector>

#include "task_store.h"

QString taskTagsText(const std::vector<std::string>& tags);

class TaskTableModel : public QAbstractTableModel {
public:
    enum Column {
        TitleColumn = 0,
        ProblemColumn,
        CategoryColumn,
        DifficultyColumn,
        TagsColumn,
        StatusColumn,
        ColumnCount
    };

    explicit TaskTableModel(QObject* parent = nullptr);

    void setTasks(std::vector<Task>* tasks);
    void reload();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    const Task* taskAt(int row) const;

private:
    std::vector<Task>* tasks_ = nullptr;
};

class TaskFilterProxyModel : public QSortFilterProxyModel {
public:
    explicit TaskFilterProxyModel(QObject* parent = nullptr);

    void setKeyword(const QString& keyword);
    void setPendingOnly(bool pendingOnly);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QString keyword_;
    bool pendingOnly_ = false;

    void notifyFilterChanged();
};
