#include "rebase.h"
#include "support/utils.h"
#include <QPushButton>

RebaseDialog::RebaseDialog(RDContext* ctx, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_context{ctx} {

    QString baseaddress = utils::to_hex(rd_get_base_address(ctx), ctx);
    this->setWindowTitle(QString{"Rebase %1"}.arg(baseaddress));

    utils::configure_hex_input(m_ui.lebaseaddress);
    m_ui.lebaseaddress->setText(utils::to_hex(rd_get_base_address(ctx), ctx));

    connect(m_ui.chkrelative, &QCheckBox::clicked, this,
            &RebaseDialog::update_address);

    connect(m_ui.lebaseaddress, &QLineEdit::textChanged, this,
            &RebaseDialog::update_address);

    this->update_address();
}

void RebaseDialog::update_address() {
    RDAddress curr_base_address = rd_get_base_address(m_context);

    bool ok = false;
    RDAddress new_base_address =
        m_ui.lebaseaddress->text().toULongLong(&ok, 16);

    if(ok) {
        bool is_relative = m_ui.chkrelative->isChecked();
        m_address = (is_relative ? curr_base_address : 0) + new_base_address;

        m_ui.lblbaseaddress->setText(
            QString{"New Base: %1"}.arg(utils::to_hex(m_address, m_context)));
    }
    else {
        m_address = curr_base_address;
        m_ui.lblbaseaddress->setText("New Base: <invalid>");
    }

    QPushButton* okbutton = m_ui.buttonbox->button(QDialogButtonBox::Ok);
    okbutton->setEnabled(ok);
}
