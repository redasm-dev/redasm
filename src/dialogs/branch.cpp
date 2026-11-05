#include "branch.h"
#include "support/utils.h"

BranchDialog::BranchDialog(RDAddress address, QWidget* parent)
    : QDialog{parent}, m_ui{this} {
    this->setWindowTitle(
        QString{"Branch Table @ %1"}.arg(utils::to_hex(address)));

    m_ui.cbxelemtype->addItem("<processor defined>", false);
    m_ui.cbxelemtype->addItem("u8", true);
    m_ui.cbxelemtype->addItem("u16", true);
    m_ui.cbxelemtype->addItem("u32", true);
    m_ui.cbxelemtype->addItem("u64", true);
    m_ui.cbxelemtype->setCurrentIndex(0);

    m_ui.cbxreftype->addItem("JUMP", RD_CR_JUMP);
    m_ui.cbxreftype->addItem("CALL", RD_CR_CALL);

    m_ui.sbcount->setMinimum(1);
    m_ui.sbcount->setValue(1);
}

QString BranchDialog::selected_element_type() const {
    if(m_ui.cbxelemtype->currentData().toBool())
        return m_ui.cbxelemtype->currentText();

    return {};
}

usize BranchDialog::selected_count() const {
    return static_cast<usize>(m_ui.sbcount->value());
}

RDXRefType BranchDialog::selected_xref_type() const {
    return static_cast<RDXRefType>(m_ui.cbxreftype->currentData().toUInt());
}
