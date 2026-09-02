#pragma once

#include "dialogs/segmentregs.h"
#include "models/functions.h"
#include "support/scheduler.h"
#include "ui/contextview.h"
#include "views/surface/isurface.h"
#include <redasm/redasm.h>

class ContextView: public QWidget {
    Q_OBJECT

public:
    explicit ContextView(RDContext* ctx, QWidget* parent = nullptr);
    ~ContextView() override;
    void show_segment_regs();
    void schedule_step();
    void invalidate();
    void handle_view_requested(ISurface::ViewRequest req,
                               std::optional<RDAddress> address = std::nullopt);

    [[nodiscard]] RDContext* context() const { return m_context; }

    [[nodiscard]] ISurface* surface() const {
        return m_ui.splitview->surface();
    }

public Q_SLOTS:
    void toggle_pause();

private Q_SLOTS:
    void check_status();

private:
    Scheduler* m_scheduler;
    ui::ContextView m_ui;
    RDContext* m_context{nullptr};
    FunctionsModel* m_functionsmodel;

private:
    SegmentRegsDialog* m_dlg_sregs{nullptr};
};
