#include "referencesto.h"
#include "support/utils.h"

ReferencesToModel::ReferencesToModel(RDContext* ctx, RDAddress address,
                                     QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx}, m_address{address} {
    m_refs = rd_get_xrefs_to(ctx, address, RD_XR_NONE);
}

RDAddress ReferencesToModel::address(const QModelIndex& index) const {
    return this->xref(index).address;
}

const RDXRef& ReferencesToModel::xref(const QModelIndex& index) const {
    if(index.row() < static_cast<int>(rd_slice_length(m_refs)))
        return rd_slice_at(m_refs, index.row());

    qFatal("Cannot get current reference");
}

QVariant ReferencesToModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        const RDXRef& r = rd_slice_at(m_refs, index.row());

        switch(index.column()) {
            case 0: return utils::to_hex(r.address);
            case 1: return rd_render_text(m_context, r.address);
            case 2: return utils::ref_getdirection(m_address, r);
            case 3: return utils::ref_typetostring(r.type);
            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole) {
        if(index.column() == 0) return Qt::AlignRight;
        if(index.column() == 1) return Qt::AlignLeft;
        return Qt::AlignCenter;
    }

    return {};
}

QVariant ReferencesToModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Address";
        case 1: return "Reference";
        case 2: return "Direction";
        case 3: return "Type";
        default: break;
    }

    return {};
}

int ReferencesToModel::columnCount(const QModelIndex&) const { return 4; }

int ReferencesToModel::rowCount(const QModelIndex&) const {
    return rd_slice_length(m_refs);
}
