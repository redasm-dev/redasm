#include "strings.h"
#include "support/utils.h"

StringsModel::StringsModel(RDContext* ctx, QObject* parent)
    : SymbolsFilterModel{ctx, RD_SYMBOL_STRING, true, 1, parent} {}

RDAddress StringsModel::address(const QModelIndex& index) const {
    return this->symbols_model()->address(index);
}

QVariant StringsModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        QModelIndex srcindex = this->mapToSource(index);
        RDContext* ctx = this->symbols_model()->context();
        RDAddress address = this->symbols_model()->address(srcindex);
        const RDSegment* s = rd_find_segment(ctx, address);

        RDType t;
        if(!rd_get_type(ctx, address, &t)) return {};

        switch(index.column()) {
            case 0: return utils::to_hex_addr(address, s);
            case 1: return utils::to_hex_addr(t.count, s);
            case 2: return QString::fromUtf8(t.name);

            case 3: {
                RDSymbol symbol = this->symbols_model()->symbol(srcindex);
                return rd_symbol_to_string(&symbol, ctx);
            }

            case 4: {
                if(rd_has_refs_to(ctx, address)) return "YES";
                return "NO";
            }

            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole) {
        if(index.column() == 0) return Qt::AlignRight;
        if((index.column() == 2) || (index.column() == 3)) return Qt::AlignLeft;
    }

    return SymbolsFilterModel::data(index, role);
}

QVariant StringsModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
            case 0: return "Address";
            case 1: return "Length";
            case 2: return "Type";
            case 3: return "String";
            case 4: return "Has XRefs";
            default: break;
        }
    }

    return SymbolsFilterModel::headerData(section, orientation, role);
}

int StringsModel::columnCount(const QModelIndex&) const { return 5; }
