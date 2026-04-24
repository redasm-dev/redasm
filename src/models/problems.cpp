#include "problems.h"
#include "support/utils.h"

ProblemsModel::ProblemsModel(RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {
    m_problems = rd_get_all_problems(ctx);
}

RDAddress ProblemsModel::address(const QModelIndex& index) const {
    if(index.row() < static_cast<int>(m_problems.length))
        return rd_slice_at(m_problems, index.row()).from_address;

    qFatal("Cannot get problem");
    return {};
}

QVariant ProblemsModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        const RDProblem* p = &rd_slice_at(m_problems, index.row());
        const RDSegment* fromseg = rd_find_segment(m_context, p->from_address);
        const RDSegment* toseg = rd_find_segment(m_context, p->address);

        switch(index.column()) {
            case 0: return utils::to_hex_addr(p->from_address, fromseg);
            case 1: return utils::to_hex_addr(p->address, toseg);
            case 2: return QString::fromUtf8(p->message);
            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole) {
        if(index.column() == 0 || index.column() == 1)
            return QVariant{Qt::AlignRight | Qt::AlignVCenter};
        return Qt::AlignLeft;
    }

    return {};
}

QVariant ProblemsModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return tr("From");
        case 1: return tr("Address");
        case 2: return tr("Problem");
        default: break;
    }

    return {};
}

int ProblemsModel::columnCount(const QModelIndex&) const { return 3; }

int ProblemsModel::rowCount(const QModelIndex&) const {
    return m_problems.length;
}
