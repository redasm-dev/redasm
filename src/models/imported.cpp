#include "imported.h"
#include "support/utils.h"

ImportedModel::ImportedModel(RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {
    m_imported = rd_get_all_imported(ctx);
}

RDAddress ImportedModel::address(const QModelIndex& index) const {
    return rd_slice_at(m_imported, index.row());
}

QVariant ImportedModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        RDAddress addr = rd_slice_at(m_imported, index.row());
        const RDSegment* seg = rd_find_segment(m_context, addr);

        RDImported imp;
        bool ok = rd_get_imported(m_context, addr, &imp);

        switch(index.column()) {
            case 0: return utils::to_hex_addr(addr, seg);

            case 1:
                return ok && imp.ordinal.has_value
                           ? QString::number(imp.ordinal.value, 16)
                           : QString{};

            case 2: return rd_get_name(m_context, addr);

            case 3:
                return ok && imp.module ? QString::fromUtf8(imp.module)
                                        : QString{};

            case 4: {
                if(rd_has_refs_to(m_context, addr)) return "YES";
                return "NO";
            }

            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole) {
        if(index.column() == 0) return Qt::AlignRight;
        return Qt::AlignLeft;
    }

    return {};
}

QVariant ImportedModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Address";
        case 1: return "Ordinal";
        case 2: return "Name";
        case 3: return "Module";
        case 4: return "Has XRefs";
        default: break;
    }

    return {};
}

int ImportedModel::columnCount(const QModelIndex&) const { return 5; }

int ImportedModel::rowCount(const QModelIndex&) const {
    return rd_slice_length(m_imported);
}
