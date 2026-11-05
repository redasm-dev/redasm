#pragma once

#include "ui/branchdialog.h"
#include <redasm/redasm.h>

class BranchDialog: public QDialog {
    Q_OBJECT

public:
    explicit BranchDialog(RDAddress address, QWidget* parent = nullptr);
    [[nodiscard]] QString selected_element_type() const;
    [[nodiscard]] usize selected_count() const;
    [[nodiscard]] RDXRefType selected_xref_type() const;

private:
    ui::BranchDialog m_ui;
};
