#include "referencesfrom.h"
#include "delegates/referencesfrom.h"
#include <QHeaderView>

ReferencesFromDialog::ReferencesFromDialog(RDContext* ctx, RDAddress address,
                                           QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_context{ctx}, m_fromaddress{address} {
    m_refsmodel = new ReferencesFromModel(ctx, address, this);

    m_ui.elvrefs->setItemDelegate(new ReferencesFromDelegate(ctx, this));
    m_ui.elvrefs->setModel(m_refsmodel);
    m_ui.elvrefs->header()->hideSection(1);
    m_ui.elvrefs->header()->hideSection(2);
    m_ui.elvrefs->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}

void ReferencesFromDialog::accept() {
    for(const auto& c : m_refsmodel->changes()) {
        switch(c.state) {
            case ChangeState::DELETED:
                rd_user_xref_del(m_context, m_fromaddress, c.original.address);
                break;

            case ChangeState::MODIFIED: {
                rd_user_xref_del(m_context, m_fromaddress, c.original.address);
                rd_user_xref_add(m_context, m_fromaddress, c.current.address,
                                 c.current.type);
                break;
            }

            case ChangeState::ADDED: {
                rd_user_xref_add(m_context, m_fromaddress, c.current.address,
                                 c.current.type);
                break;
            }

            default: break;
        }
    }

    QDialog::accept();
}
