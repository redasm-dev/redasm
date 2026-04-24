#pragma once

#include "ui/loaderdialog.h"
#include <QDialog>
#include <redasm/redasm.h>

class LoaderDialog: public QDialog {
    Q_OBJECT

public:
    explicit LoaderDialog(RDContextSlice ctxslice, QWidget* parent = nullptr);

private:
    void populate_processors() const;

public Q_SLOTS:
    void on_loader_changed(int currentrow);
    void on_processor_changed(int currentrow);

public:
    RDContext* context;
    const RDProcessorPlugin* processorplugin{nullptr};

private:
    ui::LoaderDialog m_ui;
    RDContextSlice m_contextslice{nullptr};
};
