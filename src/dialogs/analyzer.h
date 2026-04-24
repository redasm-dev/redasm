#pragma once

#include "ui/analyzerdialog.h"
#include <QDialog>
#include <QStandardItemModel>
#include <redasm/redasm.h>

class AnalyzerDialog: public QDialog {
    Q_OBJECT

public:
    explicit AnalyzerDialog(RDContext* ctx, QWidget* parent = nullptr);

private:
    void set_details_visible(bool b);
    void select_analyzers(bool select);
    void get_analyzers();

private:
    ui::AnalyzerDialog m_ui;
    RDContext* m_context;
    QStandardItemModel* m_analyzersmodel;
    RDAnalyzerItemSlice m_analyzers{};
};
