#include "segments.h"
#include "support/utils.h"

namespace {

QString get_segment_type(const RDSegment* seg) {
    QString s;
    if(seg->perm & RD_SP_R) s.append("R");
    if(seg->perm & RD_SP_W) s.append("W");
    if(seg->perm & RD_SP_X) s.append("X");
    return s;
}

} // namespace

SegmentsModel::SegmentsModel(const RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent} {
    m_segments = rd_get_all_segments(ctx);
}

RDAddress SegmentsModel::address(const QModelIndex& index) const {
    return rd_slice_at(m_segments, index.row())->start_address;
}

QVariant SegmentsModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        const RDSegment* s = rd_slice_at(m_segments, index.row());
        usize len = s->end_address - s->start_address;

        switch(index.column()) {
            case 0: return s->name;
            case 1: return utils::to_hex_addr(s->start_address, s);
            case 2: return utils::to_hex_addr(s->end_address, s);
            case 3: return utils::to_hex_addr(len, s);
            case 4: return QString::number(s->unit);
            case 5: return get_segment_type(s);
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

QVariant SegmentsModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Name";
        case 1: return "Start Address";
        case 2: return "End Address";
        case 3: return "Size";
        case 4: return "Unit";
        case 5: return "Perm";
        default: break;
    }

    return {};
}

int SegmentsModel::columnCount(const QModelIndex&) const { return 6; }

int SegmentsModel::rowCount(const QModelIndex&) const {
    return rd_slice_length(m_segments);
}
