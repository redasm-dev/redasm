#include "functions.h"

FunctionsModel::FunctionsModel(RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {
    this->resync();
}

void FunctionsModel::resync() {
    this->beginResetModel();
    m_functions = rd_get_all_functions(m_context);
    this->endResetModel();
}

RDAddress FunctionsModel::address(const QModelIndex& index) const {
    const RDFunction* f = rd_slice_at(m_functions, index.row());
    return rd_function_get_address(f);
}

QVariant FunctionsModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        const RDFunction* f = rd_slice_at(m_functions, index.row());
        RDAddress addr = rd_function_get_address(f);

        switch(index.column()) {
            case 0: return rd_get_name(m_context, addr);
            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole)
        return Qt::AlignLeft;

    return {};
}

QVariant FunctionsModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Functions";
        default: break;
    }

    return {};
}

int FunctionsModel::columnCount(const QModelIndex&) const { return 1; }

int FunctionsModel::rowCount(const QModelIndex&) const {
    return rd_slice_length(m_functions);
}
