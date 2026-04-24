#pragma once

#include "models/functions.h"
#include "ui/contextview.h"
#include "views/surface/isurface.h"
#include <redasm/redasm.h>

class ContextView: public QWidget {
    Q_OBJECT

public:
    explicit ContextView(RDContext* ctx, QWidget* parent = nullptr);
    ~ContextView() override;
    bool loop();

    [[nodiscard]] RDContext* context() const { return m_context; }

    [[nodiscard]] ISurface* surface() const {
        return m_ui.splitview->surface();
    }

public Q_SLOTS:
    void toggle_active();

private:
    void report_status();

private:
    ui::ContextView m_ui;
    RDContext* m_context{nullptr};
    const RDWorkerStatus* m_status{nullptr};
    bool m_busy{true};
    bool m_active{true};
    FunctionsModel* m_functionsmodel;
};
