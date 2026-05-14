#include "segmentregs.h"
#include "support/utils.h"

SegmentRegsModel::SegmentRegsModel(RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {}

RDAddress SegmentRegsModel::address(const QModelIndex& index) const {
    if(static_cast<usize>(index.row()) < rd_slice_length(m_registers))
        return rd_slice_at(m_registers, index.row()).address;

    qFatal("Cannot get segment register address");
    return {};
}

void SegmentRegsModel::select_register(const char* regname) {
    this->beginResetModel();
    m_registers = rd_get_all_sregval(m_context, regname);
    this->endResetModel();
}

QVariant SegmentRegsModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        const RDSegmentReg& sreg = rd_slice_at(m_registers, index.row());

        switch(index.column()) {
            case 0: return rd_to_hexaddr(m_context, sreg.address);
            case 1: return QString::fromUtf8(sreg.name);

            case 2: {
                if(sreg.has_value) return rd_to_hexaddr(m_context, sreg.value);
                break;
            }

            case 3: return utils::confidence_text(sreg.confidence);
            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole) {
        if(index.column() == 0) return Qt::AlignRight;
        if(index.column() == 1 || index.column() == 2) return Qt::AlignCenter;
        if(index.column() == 3) return Qt::AlignLeft;
    }

    return {};
}

QVariant SegmentRegsModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Address";
        case 1: return "Register";
        case 2: return "Value";
        case 3: return "Confidence";
        default: break;
    }

    return {};
}

int SegmentRegsModel::columnCount(const QModelIndex&) const { return 4; }

int SegmentRegsModel::rowCount(const QModelIndex&) const {
    return rd_slice_length(m_registers);
}
