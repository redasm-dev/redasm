#include "functions.h"
#include <cstring>

FunctionsModel::FunctionsModel(RDContext* ctx, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx} {
    rd_register_hook(ctx, RD_HOOK_FUNC, "redasm.func_adding",
                     &FunctionsModel::on_hook, this);
    rd_register_hook(ctx, RD_HOOK_FUNC, "redasm.func_added",
                     &FunctionsModel::on_hook, this);
    rd_register_hook(ctx, RD_HOOK_FUNC, "redasm.func_removing",
                     &FunctionsModel::on_hook, this);
    rd_register_hook(ctx, RD_HOOK_FUNC, "redasm.func_removed",
                     &FunctionsModel::on_hook, this);
}

RDAddress FunctionsModel::address(const QModelIndex& index) const {
    RDFunctionSlice functions = rd_get_all_functions(m_context);
    return rd_function_get_address(rd_slice_at(functions, index.row()));
}

QVariant FunctionsModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        RDFunctionSlice functions = rd_get_all_functions(m_context);
        const RDFunction* f = rd_slice_at(functions, index.row());
        RDAddress addr = rd_function_get_address(f);

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
    return static_cast<int>(rd_slice_length(rd_get_all_functions(m_context)));
}

void FunctionsModel::on_hook(RDContext* ctx, const RDHookEvent* e,
                             void* userdata) {
    RD_UNUSED(ctx);

    auto* self = static_cast<FunctionsModel*>(userdata);

    if(!std::strcmp(e->name, "redasm.func_adding"))
        self->beginInsertRows(QModelIndex{}, e->func.index, e->func.index);
    else if(!std::strcmp(e->name, "redasm.func_added"))
        self->endInsertRows();
    else if(!std::strcmp(e->name, "redasm.func_removing"))
        self->beginRemoveRows(QModelIndex{}, e->func.index, e->func.index);
    else if(!std::strcmp(e->name, "redasm.func_removed"))
        self->endRemoveRows();
}
