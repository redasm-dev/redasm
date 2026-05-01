#include "registers.h"
#include "support/utils.h"

RegistersModel::RegistersModel(RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {
    m_registers = rd_get_all_reg(ctx);
}

RDAddress RegistersModel::address(const QModelIndex& index) const {
    if(static_cast<usize>(index.row()) < rd_slice_length(m_registers))
        return rd_slice_at(m_registers, index.row()).address;

    qFatal("Cannot get reference");
    return {};
}

QVariant RegistersModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        const RDTrackedReg& tr = rd_slice_at(m_registers, index.row());

        switch(index.column()) {
            case 0: return utils::to_hex_addr(tr.address);
            case 1: return QString::fromUtf8(tr.reg.name);

            case 2: {
                if(tr.reg.has_value) return QString::number(tr.reg.value, 16);
                return "???";
            }

            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole) {
        if(index.column() == 0) return Qt::AlignRight;
    }

    return {};
}

QVariant RegistersModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Address";
        case 1: return "Register";
        case 2: return "Value";
        default: break;
    }

    return {};
}

int RegistersModel::columnCount(const QModelIndex&) const { return 3; }

int RegistersModel::rowCount(const QModelIndex&) const {
    return rd_slice_length(m_registers);
}
