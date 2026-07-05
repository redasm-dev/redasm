#pragma once

#include "ui/patchdialog.h"
#include <redasm/redasm.h>

class PatchDialog: public QDialog {
    Q_OBJECT

public:
    explicit PatchDialog(RDAddress address, QWidget* parent = nullptr);

    [[nodiscard]] QString instruction_text() const {
        return m_ui.leinstruction->text().simplified();
    }

    [[nodiscard]] bool fill_nop() const {
        return m_ui.chbxfillnop->isChecked();
    };

private Q_SLOTS:
    void validate_input();

private:
    ui::PatchDialog m_ui;
};
