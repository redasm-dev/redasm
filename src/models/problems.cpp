#include "problems.h"
#include "support/themeprovider.h"
#include "support/utils.h"

ProblemsModel::ProblemsModel(RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {
    m_problems = rd_get_all_problems(ctx);
}

RDAddress ProblemsModel::address(const QModelIndex& index) const {
    if(index.row() < static_cast<int>(m_problems.length))
        return rd_slice_at(m_problems, index.row()).target.value;

    qFatal("Cannot get problem");
    return {};
}

QVariant ProblemsModel::data(const QModelIndex& index, int role) const {
    const RDProblem* p = &rd_slice_at(m_problems, index.row());

    if(role == Qt::DisplayRole) {

        switch(index.column()) {
            case 0: return utils::to_hex(p->from.value, m_context);
            case 1: return utils::to_hex(p->target.value, m_context);
            case 2: return QString::fromUtf8(p->message);
            default: break;
        }
    }
    else if(role == Qt::ForegroundRole) {
        switch(index.column()) {
            case 0: {
                if(!p->from.is_address)
                    return theme_provider::color(RD_THEME_MUTED);
                break;
            }

            case 1: {
                if(!p->target.is_address)
                    return theme_provider::color(RD_THEME_MUTED);
                break;
            }

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
        case 1: return tr("Target");
        case 2: return tr("Problem");
        default: break;
    }

    return {};
}

int ProblemsModel::columnCount(const QModelIndex&) const { return 3; }

int ProblemsModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(m_problems.length);
}
