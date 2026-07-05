#include "patch.h"
#include <QPushButton>

PatchDialog::PatchDialog(RDAddress address, QWidget* parent)
    : QDialog{parent}, m_ui{this} {

    this->setWindowTitle(
        QString{"Patch instruction @ %1"}.arg(QString::number(address, 16)));

    connect(m_ui.leinstruction, &QLineEdit::textChanged, this,
            &PatchDialog::validate_input);

    this->validate_input();
}

void PatchDialog::validate_input() { // NOLINT
    QPushButton* btnok = m_ui.buttonbox->button(QDialogButtonBox::Ok);
    btnok->setEnabled(!m_ui.leinstruction->text().simplified().isEmpty());
}
