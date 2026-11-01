#pragma once

#include <QAbstractProxyModel>

class EditableProxyModel: public QAbstractProxyModel {
    Q_OBJECT

public:
    static constexpr int BUTTON_PRESSED_ROLE = Qt::UserRole + 100;

public:
    // clang-format off
    explicit EditableProxyModel(QObject* parent = nullptr);
    void setSourceModel(QAbstractItemModel* source_model) override;
    [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent) const override;
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
    [[nodiscard]] QModelIndex mapToSource(const QModelIndex& proxy_index) const override;
    [[nodiscard]] QModelIndex mapFromSource(const QModelIndex& source_index) const override;
    [[nodiscard]] QVariant data(const QModelIndex& proxy_index, int role) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& proxy_index) const override;
    [[nodiscard]] bool hasChildren(const QModelIndex& parent) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex& proxy_index, const QVariant& value, int role) override;
    bool insertRows(int row, int count, const QModelIndex& parent) override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;
    [[nodiscard]] QModelIndex pressed_index() const;

private Q_SLOTS:
    void on_source_rows_about_to_be_inserted(const QModelIndex& parent, int first, int last);
    void on_source_rows_inserted(const QModelIndex& parent, int first, int last);
    void on_source_rows_about_to_be_removed(const QModelIndex& parent, int first, int last);
    void on_source_rows_removed(const QModelIndex& parent, int first, int last);
    void on_source_data_changed(const QModelIndex& top_left, const QModelIndex& bottom_right, const QVector<int>& roles);
    // clang-format on

private:
    int m_pressedrow{-1};
};
