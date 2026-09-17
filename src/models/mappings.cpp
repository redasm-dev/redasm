#include "mappings.h"
#include "support/utils.h"

MappingsModel::MappingsModel(const RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {
    m_mappings = rd_get_all_mappings(ctx);
}

RDAddress MappingsModel::address(const QModelIndex& index) const {
    return rd_inputmapping_get_start(rd_slice_at(m_mappings, index.row()));
}

QVariant MappingsModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        const RDInputMapping* m = rd_slice_at(m_mappings, index.row());

        switch(index.column()) {
            case 0:
                return utils::to_hex(rd_inputmapping_get_offset(m), m_context);

            case 1:
                return utils::to_hex(rd_inputmapping_get_start(m), m_context);

            case 2: return utils::to_hex(rd_inputmapping_get_end(m), m_context);
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
    return static_cast<int>(rd_slice_length(m_mappings));
}
