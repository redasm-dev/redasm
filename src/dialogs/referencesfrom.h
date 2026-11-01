#pragma once

#include "models/referencesfrom.h"
#include "ui/referencesfromdialog.h"
#include <redasm/redasm.h>

class ReferencesFromDialog: public QDialog {
    Q_OBJECT

public:
    explicit ReferencesFromDialog(RDContext* ctx, RDAddress address,
                                  QWidget* parent = nullptr);

public Q_SLOTS:
    void accept() override;

private:
    ui::ReferencesFromDialog m_ui;
    RDContext* m_context;
    RDAddress m_fromaddress;
    ReferencesFromModel* m_refsmodel;
};
