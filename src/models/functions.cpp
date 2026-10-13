#include "functions.h"
#include <cstring>

FunctionsModel::FunctionsModel(RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {
    rd_register_hook(ctx, RD_HOOK_ADDRESS, "redasm.func_added",
                     &FunctionsModel::on_hook, this);
    rd_register_hook(ctx, RD_HOOK_ADDRESS, "redasm.func_removed",
                     &FunctionsModel::on_hook, this);

    RDAddressSlice functions = rd_get_all_functions(ctx);
    m_addresses.assign(functions.data, functions.data + functions.length);
}

RDAddress FunctionsModel::address(const QModelIndex& index) const {
    return m_addresses.at(static_cast<size_t>(index.row()));
}

QVariant FunctionsModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        RDAddress addr = this->address(index);

        switch(index.column()) {
            case 0: return QString::fromUtf8(rd_to_hexaddr(m_context, addr));
            case 1: return QString::fromUtf8(rd_get_name(m_context, addr));
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
        case 0: return "Address";
        case 1: return "Function";
        default: break;
    }

    return {};
}

int FunctionsModel::columnCount(const QModelIndex&) const { return 2; }

int FunctionsModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(m_addresses.size());
}

void FunctionsModel::on_hook(RDContext* ctx, const RDHookEvent* e,
                             void* userdata) {
    RD_UNUSED(ctx);

    auto* self = static_cast<FunctionsModel*>(userdata);
    RDAddress address = e->addr.address;
    auto& v = self->m_addresses;
    auto it = std::lower_bound(v.begin(), v.end(), address);
    int row = static_cast<int>(it - v.begin());

    if(!std::strcmp(e->name, "redasm.func_added")) {
        if(it != v.end() && *it == address) return; // already in the snapshot

        self->beginInsertRows(QModelIndex{}, row, row);
        v.insert(it, address);
        self->endInsertRows();
    }
    else if(!std::strcmp(e->name, "redasm.func_removed")) {
        if(it == v.end() || *it != address) return;

        self->beginRemoveRows(QModelIndex{}, row, row);
        v.erase(it);
        self->endRemoveRows();
    }
}
