#pragma once

#include "views/split/view.h"
#include "views/surface/isurface.h"
#include <QStackedWidget>
#include <redasm/redasm.h>

class SurfaceSplitDelegate: public SplitDelegate {
    Q_OBJECT

public:
    explicit SurfaceSplitDelegate(RDContext* ctx, QObject* parent = nullptr);
    QWidget* create_widget(SplitWidget* split, SplitWidget* current) override;

private:
    RDContext* m_context;
};

class SurfaceSplitView: public SplitView {
    Q_OBJECT

public:
    explicit SurfaceSplitView(RDContext* ctx, QWidget* parent = nullptr);
    [[nodiscard]] ISurface* surface() const;
};
