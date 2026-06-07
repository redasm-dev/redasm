#pragma once

#include "ui/loaderdialog.h"
#include <QDialog>
#include <redasm/redasm.h>

class LoaderDialog: public QDialog {
    Q_OBJECT

public:
    explicit LoaderDialog(RDContextSlice ctxslice, QWidget* parent = nullptr);

private Q_SLOTS:
    void on_loader_changed(int currentrow);
    void on_processor_changed(int currentrow);
    void update_min_string();
    void update_entry_point();
    void update_address();
    void update_offset();
    void validate_fields();

private:
    void populate_processors() const;

public:
    RDContext* context;
    const RDProcessorPlugin* processorplugin{nullptr};
    RDLoadAddressing addressing{};

private:
    ui::LoaderDialog m_ui;
    RDContextSlice m_contextslice{};
};
