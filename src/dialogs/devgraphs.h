#pragma once

#include "models/functions.h"
#include "ui/devgraphsdialog.h"
#include <redasm/redasm.h>

class DevGraphsDialog: public QDialog {
    Q_OBJECT

public:
    explicit DevGraphsDialog(RDContext* ctx, QWidget* parent = nullptr);

private Q_SLOTS:
    void show_dot(const QModelIndex& index);
    void copy_graph_hashes();
    void copy_functions();

private:
    ui::DevGraphsDialog m_ui;
    RDContext* m_context;
    FunctionsModel* m_functionsmodel;
    QString m_currentgraph{};
    u32 m_currenthash{};
};
