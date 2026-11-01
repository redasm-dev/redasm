#include "editable.h"
#include "support/themeprovider.h"
#include <QColor>

EditableProxyModel::EditableProxyModel(QObject* parent)
    : QAbstractProxyModel{parent} {}

void EditableProxyModel::setSourceModel(QAbstractItemModel* source_model) {
    m_pressedrow = -1;

    if(this->sourceModel())
        disconnect(this->sourceModel(), nullptr, this, nullptr);

    QAbstractProxyModel::setSourceModel(source_model);
    if(!source_model) return;

    connect(source_model, &QAbstractItemModel::rowsAboutToBeInserted, this,
            &EditableProxyModel::on_source_rows_about_to_be_inserted);
    connect(source_model, &QAbstractItemModel::rowsInserted, this,
            &EditableProxyModel::on_source_rows_inserted);
    connect(source_model, &QAbstractItemModel::rowsAboutToBeRemoved, this,
            &EditableProxyModel::on_source_rows_about_to_be_removed);
    connect(source_model, &QAbstractItemModel::rowsRemoved, this,
            &::EditableProxyModel::on_source_rows_removed);
    connect(source_model, &QAbstractItemModel::dataChanged, this,
            &::EditableProxyModel::on_source_data_changed);
}

int EditableProxyModel::rowCount(const QModelIndex& parent) const {
    if(!this->sourceModel() || parent.isValid()) return 0;
    return this->sourceModel()->rowCount() + 1;
}

int EditableProxyModel::columnCount(const QModelIndex& parent) const {
    if(!this->sourceModel() || parent.isValid()) return 0;
    return this->sourceModel()->columnCount() + 1;
}

QModelIndex EditableProxyModel::index(int row, int column,
                                      const QModelIndex& parent) const {
    if(parent.isValid() || row < 0 || row >= this->rowCount(parent) ||
       column < 0 || column >= this->columnCount(parent))
        return {};

    return this->createIndex(row, column);
}

QModelIndex EditableProxyModel::parent(const QModelIndex& child) const {
    Q_UNUSED(child);
    return {};
}

QModelIndex
EditableProxyModel::mapToSource(const QModelIndex& proxy_index) const {
    if(!sourceModel() || !proxy_index.isValid()) return {};

    if(proxy_index.row() >= this->sourceModel()->rowCount() ||
       proxy_index.column() >= this->sourceModel()->columnCount())
        return {};

    return this->sourceModel()->index(proxy_index.row(), proxy_index.column());
}

QModelIndex
EditableProxyModel::mapFromSource(const QModelIndex& source_index) const {
    if(!source_index.isValid()) return {};
    return this->index(source_index.row(), source_index.column(),
                       source_index.parent());
}

QVariant EditableProxyModel::data(const QModelIndex& proxy_index,
                                  int role) const {
    if(!this->sourceModel() || !proxy_index.isValid()) return {};

    if(role == BUTTON_PRESSED_ROLE) return proxy_index.row() == m_pressedrow;

    int real_rows = this->sourceModel()->rowCount();
    int real_cols = this->sourceModel()->columnCount();

    if(proxy_index.row() == real_rows) {
        if(proxy_index.column() == 0) {
            if(role == Qt::DisplayRole) return "+ Add New Element…";
            if(role == Qt::EditRole) return QString{};
        }

        if(role == Qt::ForegroundRole)
            return theme_provider::color(RD_THEME_MUTED);

        return {};
    }

    if(proxy_index.column() == real_cols) return {};

    return this->sourceModel()->data(this->mapToSource(proxy_index), role);
}

Qt::ItemFlags EditableProxyModel::flags(const QModelIndex& proxy_index) const {
    if(!proxy_index.isValid()) return Qt::NoItemFlags;

    int real_rows = this->sourceModel()->rowCount();
    int real_cols = this->sourceModel()->columnCount();

    if(real_rows == proxy_index.row()) {
        if(proxy_index.column() == 0) {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable |
                   Qt::ItemIsEditable;
        }

        return Qt::ItemIsEnabled;
    }

    if(proxy_index.column() == real_cols)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return this->sourceModel()->flags(this->mapToSource(proxy_index));
}

bool EditableProxyModel::hasChildren(const QModelIndex& parent) const {
    return !parent.isValid() && this->rowCount(parent) > 0;
}

QVariant EditableProxyModel::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const {
    if(!this->sourceModel()) return {};

    if(orientation == Qt::Horizontal) {
        if(section < this->sourceModel()->columnCount())
            return this->sourceModel()->headerData(section, orientation, role);
        return {}; // button column
    }

    if(section < this->sourceModel()->rowCount())
        return this->sourceModel()->headerData(section, orientation, role);
    return {}; // placeholder row
}

bool EditableProxyModel::setData(const QModelIndex& proxy_index,
                                 const QVariant& value, int role) {
    if(!this->sourceModel() || !proxy_index.isValid()) return false;

    if(role == BUTTON_PRESSED_ROLE) {
        m_pressedrow = value.toBool() ? proxy_index.row() : -1;
        emit dataChanged(proxy_index, proxy_index);
        return true;
    }

    QModelIndex source_idx = this->mapToSource(proxy_index);
    if(!source_idx.isValid()) return false;

    return this->sourceModel()->setData(source_idx, value, role);
}
bool EditableProxyModel::insertRows(int row, int count,
                                    const QModelIndex& parent) {
    if(!this->sourceModel() || parent.isValid()) return false;

    return this->sourceModel()->insertRows(row, count, {});
}

bool EditableProxyModel::removeRows(int row, int count,
                                    const QModelIndex& parent) {
    if(!this->sourceModel() || parent.isValid()) return false;

    int real_rows = sourceModel()->rowCount();
    if(row + count > real_rows) return false;

    return this->sourceModel()->removeRows(row, count, {});
}

QModelIndex EditableProxyModel::pressed_index() const {
    if(m_pressedrow == -1) return {};
    return this->index(m_pressedrow, this->columnCount({}) - 1, {});
}

void EditableProxyModel::on_source_rows_about_to_be_inserted(
    const QModelIndex& parent, int first, int last) {
    this->beginInsertRows(parent, first, last);
}

void EditableProxyModel::on_source_rows_inserted(const QModelIndex& parent,
                                                 int first, int last) {
    Q_UNUSED(first);
    Q_UNUSED(last);

    this->endInsertRows();

    int new_insert_row = this->sourceModel()->rowCount();
    QModelIndex proxy_insert_idx = this->index(new_insert_row, 0, parent);
    Q_EMIT dataChanged(proxy_insert_idx, proxy_insert_idx);
}

void EditableProxyModel::on_source_rows_about_to_be_removed(
    const QModelIndex& parent, int first, int last) {
    this->beginRemoveRows(parent, first, last);
}

void EditableProxyModel::on_source_rows_removed(const QModelIndex& parent,
                                                int first, int last) {
    Q_UNUSED(first);
    Q_UNUSED(last);

    this->endRemoveRows();

    int new_insert_row = this->sourceModel()->rowCount();
    QModelIndex proxy_insert_idx = this->index(new_insert_row, 0, parent);
    Q_EMIT dataChanged(proxy_insert_idx, proxy_insert_idx);
}

void EditableProxyModel::on_source_data_changed(const QModelIndex& top_left,
                                                const QModelIndex& bottom_right,
                                                const QVector<int>& roles) {
    Q_EMIT dataChanged(this->mapFromSource(top_left),
                       this->mapFromSource(bottom_right), roles);
}
