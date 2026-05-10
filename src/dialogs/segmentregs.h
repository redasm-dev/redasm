#pragma once

#include "models/segmentregs.h"
#include "ui/segmentregsdialog.h"
#include <QDialog>
#include <redasm/redasm.h>

class SegmentRegsDialog: public QDialog {
    Q_OBJECT

public:
    explicit SegmentRegsDialog(RDContext* ctx, QWidget* parent = nullptr);
    void fetch_registers();

private Q_SLOTS:
    void show_register(int idx);

private:
    ui::SegmentRegsDialog m_ui;
    RDContext* m_context;
    SegmentRegsModel* m_model;
};
