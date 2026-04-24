#pragma once

#include <QAbstractListModel>
#include <QStringList>

class RecentFilesModel: public QAbstractListModel {
    Q_OBJECT

public:
    explicit RecentFilesModel(QObject* parent = nullptr);
    [[nodiscard]] const QString& file_path(const QModelIndex& index) const;
    void update();

public:
    [[nodiscard]] int rowCount(const QModelIndex&) const override;
    [[nodiscard]] int columnCount(const QModelIndex&) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role) const override;

private:
    QStringList m_recents;
};
