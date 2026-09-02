#pragma once

#include "hexview/flagsdelegate.h"
#include "ui/memorymapdialog.h"
#include <QDialog>
#include <redasm/redasm.h>

class MemoryMapDialog: public QDialog {
    Q_OBJECT

public:
    explicit MemoryMapDialog(RDContext* ctx, QWidget* parent = nullptr);

private Q_SLOTS:
    void show_memory(int idx);

private:
    ui::MemoryMapDialog m_ui;
    FlagsDelegate* m_flagsdelegate;
    RDContext* m_context;
};
