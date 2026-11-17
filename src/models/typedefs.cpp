#include "typedefs.h"

namespace {

QString _typedef_kind_str(const RDTypeDef* tdef) {
    switch(rd_typedef_kind(tdef)) {
        case RD_TKIND_PRIM: return "PRIMITIVE";
        case RD_TKIND_STRUCT: return "STRUCT";
        case RD_TKIND_UNION: return "UNION";
        case RD_TKIND_ENUM: return "ENUM";
        case RD_TKIND_FUNC: return "FUNC";
        default: break;
    }

    return QString{};
}

} // namespace

TypedefsModel::TypedefsModel(RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {
    m_typedefs = rd_get_all_type_defs(ctx);
}

const RDTypeDef* TypedefsModel::type_def(const QModelIndex& index) const {
    if(index.row() < static_cast<int>(rd_slice_length(m_typedefs)))
        return rd_slice_at(m_typedefs, index.row());

    qFatal("Cannot get typedef");
    return {};
}

QVariant TypedefsModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        const RDTypeDef* tdef = rd_slice_at(m_typedefs, index.row());

        switch(index.column()) {
            case 0: return QString::fromUtf8(rd_typedef_name(tdef));
            case 1: return _typedef_kind_str(tdef);
            case 2: return QString::number(rd_typedef_size(tdef));
            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole) {
        if(index.column() == 0) return Qt::AlignLeft;
        return Qt::AlignCenter;
    }

    return {};
}

QVariant TypedefsModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return tr("Name");
        case 1: return tr("Kind");
        case 2: return tr("Size");
        default: break;
    }

    return {};
}

int TypedefsModel::columnCount(const QModelIndex&) const { return 3; }

int TypedefsModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(rd_slice_length(m_typedefs));
}
