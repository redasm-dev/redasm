#include "segments.h"
#include "support/utils.h"

namespace {

QString get_segment_type(const RDSegment* seg) {
    RDSegmentPerm perm = rd_segment_get_perm(seg);

    QString s;
    if(perm & RD_SP_R) s.append("R");
    if(perm & RD_SP_W) s.append("W");
    if(perm & RD_SP_X) s.append("X");
    return s;
}

} // namespace

SegmentsModel::SegmentsModel(const RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {
    m_segments = rd_get_all_segments(ctx);
}

RDAddress SegmentsModel::address(const QModelIndex& index) const {
    return rd_segment_get_start(rd_slice_at(m_segments, index.row()));
}

QVariant SegmentsModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        const RDSegment* s = rd_slice_at(m_segments, index.row());

        usize bank_id = 0;
        RDAddress bank_base = 0;
        bool is_bank = rd_segment_get_bank(s, &bank_id, &bank_base, m_context);

        switch(index.column()) {
            case 0: return QString::fromUtf8(rd_segment_get_name(s));
            case 1: return utils::to_hex(rd_segment_get_start(s), m_context);
            case 2: return utils::to_hex(rd_segment_get_end(s), m_context);
            case 3: return utils::to_hex(rd_segment_get_size(s), m_context);
            case 4: return get_segment_type(s);
            case 5: return utils::to_hex(bank_id, nullptr);
            case 6: return utils::to_hex(bank_base, m_context);

            case 7: {
                return is_bank
                           ? utils::to_hex(rd_segment_get_start(s) - bank_base,
                                           m_context)
                           : QString{};
            }

            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole) {
        if(index.column() == 0) return Qt::AlignRight;
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
        case 4: return "Perm";
        case 5: return "Bank Id";
        case 6: return "Bank Base";
        case 7: return "Bank Address";
        default: break;
    }

    return {};
}

int SegmentsModel::columnCount(const QModelIndex&) const { return 8; }

int SegmentsModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(rd_slice_length(m_segments));
}
