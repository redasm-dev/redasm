#include "referencesfrom.h"
#include "support/utils.h"

ReferencesFromModel::ReferencesFromModel(RDContext* ctx, RDAddress address,
                                         QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx}, m_address{address} {

    // create a mutable copy
    RDXRefSlice refs = rd_get_xrefs_from(ctx, address, RD_XR_NONE);

    QList<RDXRef> items;
    items.reserve(rd_slice_length(refs));

    const RDXRef* xref;
    rd_slice_each(xref, refs) { items.push_back(*xref); }

    m_refs.reset(items);
}

RDAddress ReferencesFromModel::address(const QModelIndex& index) const {
    return this->xref(index).address;
}

RDXRef ReferencesFromModel::xref(const QModelIndex& index) const {
    if(index.row() < m_refs.size()) return m_refs.at(index.row());
    qFatal("Cannot get current reference");
}

QVariant ReferencesFromModel::data(const QModelIndex& index, int role) const {
    if(!index.isValid()) return {};

    if(role == Qt::DisplayRole) {
        const RDXRef& r = m_refs.at(index.row());

        switch(index.column()) {
            case 0: return utils::to_hex(r.address);
            case 1: return rd_render_text(m_context, r.address);
            case 2: return utils::ref_getdirection(m_address, r);
            case 3: return utils::ref_typetostring(r.type);
            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole) {
        if(index.column() == 0) return Qt::AlignRight;
        if(index.column() == 1) return Qt::AlignLeft;
        return Qt::AlignCenter;
    }

    return {};
}

bool ReferencesFromModel::setData(const QModelIndex& index,
                                  const QVariant& value, int role) {
    if(!index.isValid() || role != Qt::EditRole) return false;

    RDXRef& r = m_refs.at(index.row());

    if(index.column() == 0)
        r.address = value.toULongLong();
    else if(index.column() == 3)
        r.type = static_cast<RDXRefType>(value.toInt());
    else
        return false;

    Q_EMIT dataChanged(index, index, {role});
    return true;
}

bool ReferencesFromModel::insertRows(int row, int count,
                                     const QModelIndex& parent) {
    if(count < 1 || row < 0 || row > static_cast<int>(m_refs.size()))
        return false;

    this->beginInsertRows(parent, row, row + count - 1);
    m_refs.insert(row, count, {0, RD_DR_READ, RD_CONFIDENCE_USER});
    this->endInsertRows();
    return true;
}

bool ReferencesFromModel::removeRows(int row, int count,
                                     const QModelIndex& parent) {
    if(count < 1 || row < 0 || (row + count) > static_cast<int>(m_refs.size()))
        return false;

    this->beginRemoveRows(parent, row, row + count - 1);
    m_refs.remove(row, count);
    this->endRemoveRows();
    return true;
}

QVariant ReferencesFromModel::headerData(int section,
                                         Qt::Orientation orientation,
                                         int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Address";
        case 1: return "Reference";
        case 2: return "Direction";
        case 3: return "Type";
        default: break;
    }

    return {};
}

int ReferencesFromModel::columnCount(const QModelIndex&) const { return 4; }

int ReferencesFromModel::rowCount(const QModelIndex&) const {
    return m_refs.size();
}

Qt::ItemFlags ReferencesFromModel::flags(const QModelIndex& index) const {
    if(index.column() == 0 || index.column() == 3)
        return Qt::ItemIsEditable | QAbstractListModel::flags(index);

    return QAbstractListModel::flags(index);
}
