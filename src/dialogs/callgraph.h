#pragma once

#include "ui/callgraphdialog.h"

class CallGraphDialog: public QDialog {
    Q_OBJECT

public:
    explicit CallGraphDialog(RDContext* ctx, RDAddress address,
                             QWidget* parent = nullptr);

Q_SIGNALS:
    void address_requested(RDAddress address);

private:
    ui::CallGraphDialog m_ui;
};
