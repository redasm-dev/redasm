#include "exported.h"
#include "support/utils.h"

ExportedModel::ExportedModel(RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {
    m_exported = rd_get_all_exported(ctx);
}

RDAddress ExportedModel::address(const QModelIndex& index) const {
    return rd_slice_at(m_exported, index.row());
}

QVariant ExportedModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        RDAddress addr = rd_slice_at(m_exported, index.row());
        const RDSegment* seg = rd_find_segment(m_context, addr);

        switch(index.column()) {
            case 0: return utils::to_hex_addr(addr, seg);
            case 1: return rd_get_name(m_context, addr);

            case 2: {
                if(rd_has_refs_to(m_context, addr)) return "YES";
                return "NO";
            }

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

QVariant ExportedModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Address";
        case 1: return "Name";
        case 2: return "Has XRefs";
        default: break;
    }

    return {};
}

int ExportedModel::columnCount(const QModelIndex&) const { return 3; }

int ExportedModel::rowCount(const QModelIndex&) const {
    return rd_slice_length(m_exported);
}
