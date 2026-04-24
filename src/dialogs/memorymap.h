#pragma once

#include "ui/memorymapdialog.h"
#include <QDialog>
#include <redasm/redasm.h>

class MemoryMapDialog: public QDialog {
    Q_OBJECT

public:
    explicit MemoryMapDialog(const RDContext* ctx, QWidget* parent = nullptr);

private Q_SLOTS:
    void show_memory(int idx);

private:
    ui::MemoryMapDialog m_ui;
    const RDContext* m_context;
};
