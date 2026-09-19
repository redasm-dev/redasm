#pragma once

#include "ui/rebasedialog.h"
#include <redasm/redasm.h>

class RebaseDialog: public QDialog {
    Q_OBJECT

public:
    explicit RebaseDialog(RDContext* ctx, QWidget* parent = nullptr);
    [[nodiscard]] RDAddress address() const { return m_address; }

private Q_SLOTS:
    void update_address();

private:
    ui::RebaseDialog m_ui;
    RDContext* m_context;
    RDAddress m_address{};
};
