#include "references.h"
#include "support/utils.h"

namespace {

QString reftype_tostring(const RDXRef& r) {
    switch(r.type) {
        case RD_DR_READ: return "Read";
        case RD_DR_WRITE: return "Write";
        case RD_DR_ADDRESS: return "Address";
        case RD_CR_CALL: return "Call";
        case RD_CR_JUMP: return "Jump";
        default: break;
    }

    return {};
}

QString ref_getdirection(RDAddress address, const RDXRef& r) {
    if(r.address > address) return "Down";
    if(r.address < address) return "Up";
    return "---";
}

} // namespace

ReferencesModel::ReferencesModel(RDContext* ctx, RDAddress address,
                                 QObject* parent)
    : QAbstractListModel{parent}, m_context{ctx}, m_address{address} {
    m_refs = rd_get_xrefs_to(ctx, address);
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
            case 0: return utils::to_hex_addr(r.address);
            case 1: return reftype_tostring(r);
            case 2: return ref_getdirection(m_address, r);
            case 3: return rd_render_text(m_context, r.address);
            default: break;
        }
    }
    else if(role == Qt::TextAlignmentRole) {
        if(index.column() == 0) return Qt::AlignRight;
        if(index.column() == 3) return Qt::AlignLeft;
    }

    return {};
}

QVariant ReferencesModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const {
    if(orientation == Qt::Vertical || role != Qt::DisplayRole) return {};

    switch(section) {
        case 0: return "Address";
        case 1: return "Type";
        case 2: return "Direction";
        case 3: return "Reference";
        default: break;
    }

    return {};
}

int ReferencesModel::columnCount(const QModelIndex&) const { return 4; }

int ReferencesModel::rowCount(const QModelIndex&) const {
    return rd_slice_length(m_refs);
}
