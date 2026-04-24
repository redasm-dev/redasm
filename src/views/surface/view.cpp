#include "view.h"
#include "support/surfacerenderer.h"
#include "support/themeprovider.h"
#include <QKeyEvent>
#include <redasm/redasm.h>

SurfaceView::SurfaceView(RDContext* ctx, QWidget* parent): QSplitter{parent} {
    this->setStyleSheet("QSplitter::handle { background-color: " +
                        themeprovider::color(RD_THEME_SEEK).name() + "; }");

    this->setFocusPolicy(Qt::NoFocus);

    m_surfacelisting = new SurfaceListing(ctx);
    m_surfacepath = new SurfacePath(m_surfacelisting);

    this->addWidget(m_surfacepath);
    this->addWidget(m_surfacelisting);

    // Shrink the left side
    auto w = qCeil(surface_renderer::cell_width()) * 8;
    this->setSizes({w, this->width() - w});
    this->setHandleWidth(4);

    connect(m_surfacelisting, &SurfaceListing::history_updated, this,
            &SurfaceView::history_updated);

    connect(m_surfacelisting, &SurfaceListing::switch_view, this,
            &SurfaceView::switch_view);
}
