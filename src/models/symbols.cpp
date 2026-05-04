#include "symbols.h"
#include "support/themeprovider.h"
#include "support/utils.h"

namespace {

QColor get_foreground_color(const RDSymbol& sym) {
    switch(sym.kind) {
        case RD_SYMBOL_SEGMENT: return theme_provider::color(RD_THEME_SEGMENT);
        case RD_SYMBOL_FUNCTION:
            return theme_provider::color(RD_THEME_FUNCTION);
        case RD_SYMBOL_TYPE: return theme_provider::color(RD_THEME_TYPE);
        case RD_SYMBOL_STRING: return theme_provider::color(RD_THEME_STRING);
        default: break;
    }

    return theme_provider::color(RD_THEME_FOREGROUND);
}

} // namespace

SymbolsModel::SymbolsModel(RDContext* ctx, bool autoalign, QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx}, m_autoalign{autoalign} {
    m_colsymbol = "Symbol";
    this->resync();
}

RDAddress SymbolsModel::address(const QModelIndex& index) const {
    if(index.row() < static_cast<int>(m_symbols.length))
        return rd_slice_at(m_symbols, index.row()).address;

    qFatal("Cannot get symbol address");
    return {};
}

void SymbolsModel::resync() {
    this->beginResetModel();
    m_symbols = rd_get_all_symbols(m_context);
    this->endResetModel();
}

QVariant SymbolsModel::data(const QModelIndex& index, int role) const {
    if(index.row() >= static_cast<int>(rd_slice_length(m_symbols))) return {};

    RDSymbol sym = rd_slice_at(m_symbols, index.row());

    if(role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0: return utils::to_hex_addr(sym.address);
            case 1: return this->get_symbol_kind(sym.kind);
            case 2: return rd_symbol_to_string(&sym, m_context);

            case 3: {
                if(rd_has_refs_to(m_context, sym.address)) return "YES";
                return "NO";
            }

            default: break;
        }
    }
    else if(m_autoalign && role == Qt::TextAlignmentRole) {
        if(index.column() == 0)
            return QVariant{Qt::AlignRight | Qt::AlignVCenter};
        if(index.column() == 1) return Qt::AlignCenter;
        return Qt::AlignLeft;
    }
    else if(role == Qt::ForegroundRole) {
        if(m_highlightaddress && index.column() == 0)
            return theme_provider::color(RD_THEME_LOCATION);
        if(m_highlightsymbol && index.column() == 2)
            return get_foreground_color(sym);
    }
    else if(role == Qt::UserRole)
        return QVariant::fromValue(sym.kind);

    return {};
}

QVariant SymbolsModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Address";
        case 1: return "Type";
        case 2: return m_colsymbol;
        case 3: return "Has XRefs";
        default: break;
    }

    return {};
}

int SymbolsModel::columnCount(const QModelIndex&) const { return 4; }

int SymbolsModel::rowCount(const QModelIndex&) const {
    return rd_slice_length(m_symbols);
}

QString SymbolsModel::get_symbol_kind(u32 t) const {
    switch(t) {
        case RD_SYMBOL_SEGMENT: return "SEGMENT";
        case RD_SYMBOL_FUNCTION: return "FUNCTION";
        case RD_SYMBOL_TYPE: return "TYPE";
        case RD_SYMBOL_STRING: return "STRING";
        case RD_SYMBOL_IMPORTED: return "IMPORTED";
        case RD_SYMBOL_EXPORTED: return "EXPORTED";
        default: break;
    }

    return {};
}
