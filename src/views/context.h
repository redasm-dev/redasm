#pragma once

#include "dialogs/segmentregs.h"
#include "models/functions.h"
#include "ui/contextview.h"
#include "views/surface/isurface.h"
#include <QElapsedTimer>
#include <redasm/redasm.h>

class ContextView: public QWidget {
    Q_OBJECT

public:
    explicit ContextView(RDContext* ctx, QWidget* parent = nullptr);
    ~ContextView() override;
    void show_segment_regs();
    void schedule_step();

    [[nodiscard]] RDContext* context() const { return m_context; }

    [[nodiscard]] ISurface* surface() const {
        return m_ui.splitview->surface();
    }

public Q_SLOTS:
    void toggle_pause();

private:
    void check_status();

private:
    ui::ContextView m_ui;
    RDContext* m_context{nullptr};
    RDWorkerStatus m_status{};
    bool m_pause{false};
    FunctionsModel* m_functionsmodel;
    QElapsedTimer m_burst_timer;
    QElapsedTimer m_notify_timer;

private:
    SegmentRegsDialog* m_dlg_sregs{nullptr};
};
