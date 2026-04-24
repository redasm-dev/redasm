#include "mappings.h"
#include "support/utils.h"

MappingsModel::MappingsModel(const RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent} {
    m_mappings = rd_get_all_mappings(ctx);
}

RDAddress MappingsModel::address(const QModelIndex& index) const {
    return rd_slice_at(m_mappings, index.row()).start_address;
}

QVariant MappingsModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        RDInputMapping m = rd_slice_at(m_mappings, index.row());

        switch(index.column()) {
            case 0: return utils::to_hex_addr(m.offset);
            case 1: return utils::to_hex_addr(m.start_address);
            case 2: return utils::to_hex_addr(m.end_address);
            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole) {
        if(index.column() == 0)
            return QVariant{Qt::AlignRight | Qt::AlignVCenter};
        return Qt::AlignCenter;
    }

    return {};
}

QVariant MappingsModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Offset";
        case 1: return "Start Address";
        case 2: return "End Address";
        default: break;
    }

    return {};
}

int MappingsModel::columnCount(const QModelIndex&) const { return 4; }

int MappingsModel::rowCount(const QModelIndex&) const {
    return rd_slice_length(m_mappings);
}
