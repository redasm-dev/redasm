#pragma once

#include "models/loaderoptions.h"
#include "ui/loaderdialog.h"
#include <QDialog>
#include <redasm/redasm.h>

class LoaderDialog: public QDialog {
    Q_OBJECT

public:
    explicit LoaderDialog(RDTestResultSlice ctxslice,
                          QWidget* parent = nullptr);

private Q_SLOTS:
    void on_loader_changed(int currentrow);
    void on_processor_changed(int currentrow);
    void update_min_string();
    void update_open_mode();
    void update_base_address();
    void update_entry_point();
    void update_address();
    void update_offset();
    void validate_fields();

private:
    void populate_processors() const;

public:
    RDTestResult* sel_test{nullptr};
    RDAcceptParams accept_params{};

private:
    ui::LoaderDialog m_ui;
    LoaderOptionsModel* m_loaderoptionsmodel;
    RDTestResultSlice m_testslice{};
};
