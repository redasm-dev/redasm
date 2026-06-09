#include "references.h"
#include "support/utils.h"

namespace {

QString reftype_tostring(const RDXRef& r) {
    switch(r.type) {
        case RD_DR_READ: return "READ";
        case RD_DR_WRITE: return "WRITE";
        case RD_DR_ADDRESS: return "ADDRESS";
        case RD_CR_CALL: return "CALL";
        case RD_CR_JUMP: return "JUMP";
        default: break;
    }

    return {};
}

QString ref_getdirection(RDAddress address, const RDXRef& r) {
    if(r.address > address) return "DOWN";
    if(r.address < address) return "UP";
    return "---";
}

} // namespace

ReferencesModel::ReferencesModel(RDContext* ctx, RDAddress address,
                                 QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx}, m_address{address} {
    m_refs = rd_get_xrefs_to(ctx, address, RD_XR_NONE);
}

RDAddress ReferencesModel::address(const QModelIndex& index) const {
    if(static_cast<usize>(index.row()) < rd_slice_length(m_refs))
        return rd_slice_at(m_refs, index.row()).address;

    qFatal("Cannot get reference");
    return {};
}

QVariant ReferencesModel::data(const QModelIndex& index, int role) const {
    if(role == Qt::DisplayRole) {
        const RDXRef& r = rd_slice_at(m_refs, index.row());

        switch(index.column()) {
            case 0: return utils::to_hex(r.address);
            case 1: return rd_render_text(m_context, r.address);
            case 2: return ref_getdirection(m_address, r);
            case 3: return reftype_tostring(r);
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

QVariant ReferencesModel::headerData(int section, Qt::Orientation orientation,
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

int ReferencesModel::columnCount(const QModelIndex&) const { return 4; }

int ReferencesModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(rd_slice_length(m_refs));
}
