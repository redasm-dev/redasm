#pragma once

#include "support/scheduler.h"
#include "views/split/view.h"
#include "views/surface/isurface.h"
#include <QStackedWidget>
#include <redasm/redasm.h>

class SurfaceSplitDelegate: public SplitDelegate {
    Q_OBJECT

public:
    explicit SurfaceSplitDelegate(RDContext* ctx, Scheduler* scheduler,
                                  QObject* parent = nullptr);
    QWidget* create_widget(SplitWidget* current, SplitWidget* split) override;

private:
    RDContext* m_context;
    Scheduler* m_scheduler;
};

class SurfaceSplitView: public SplitView {
    Q_OBJECT

public:
    explicit SurfaceSplitView(RDContext* ctx, Scheduler* scheduler,
                              QWidget* parent = nullptr);
    [[nodiscard]] ISurface* surface() const;
};
