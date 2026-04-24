#include "surfacerenderer.h"
#include "support/settings.h"
#include "support/themeprovider.h"
#include <QPainter>

namespace surface_renderer {

namespace {

QFont g_font;
qreal g_cell_w = 0;
qreal g_cell_h = 0;

void render_row(QPainter* p, RDRowSlice row, qreal y) {
    for(usize j = 0; j < rd_slice_length(row); j++) {
        RDCell c = rd_slice_at(row, j);
        QColor fg = themeprovider::color(c.fg);
        QColor bg = themeprovider::color(c.bg);

        Q_ASSERT(fg.isValid());
        Q_ASSERT(bg.isValid());

        QRectF r(j * g_cell_w, y, g_cell_w, g_cell_h);

        p->fillRect(r, bg);
        p->setPen(fg);
        p->drawText(r, Qt::AlignCenter, QChar{c.ch});
    }
}

} // namespace

void init() {
    g_font = REDasmSettings::font();

    QFontMetricsF fm{g_font};
    g_cell_w = fm.horizontalAdvance(" ");
    g_cell_h = fm.height();
}

RDSurfacePos hit_test(QPoint pt) {
    return {
        qFloor(pt.y() / g_cell_h),
        qFloor(pt.x() / g_cell_w),
    };
}

qreal cell_width() { return surface_renderer::g_cell_w; }
qreal cell_height() { return surface_renderer::g_cell_h; }
QFont get_font() { return surface_renderer::g_font; }

void render(QPainter* p, RDSurface* surface, usize start, usize n) {
    p->setFont(surface_renderer::g_font);

    for(usize i = 0; i < n; i++) {
        RDRowSlice row = rd_surface_get_row(surface, start + i);
        qreal y = i * surface_renderer::g_cell_h;

        for(usize j = 0; j < rd_slice_length(row); j++) {
            RDCell c = rd_slice_at(row, j);
            QColor fg = themeprovider::color(c.fg);
            QColor bg = themeprovider::color(c.bg);

            Q_ASSERT(fg.isValid());
            Q_ASSERT(bg.isValid());

            QRectF r(j * g_cell_w, y, g_cell_w, g_cell_h);

            p->fillRect(r, bg);
            p->setPen(fg);
            p->drawText(r, Qt::AlignCenter, QChar{c.ch});
        }
    }
}

void render_block(QPainter* p, RDSurfaceGraph* surface, usize start, usize n) {
    p->setFont(g_font);

    for(usize i = 0; i < n; i++) {
        RDRowSlice row = rd_surfacegraph_get_row(surface, start + i);
        qreal y = i * surface_renderer::g_cell_h;
        surface_renderer::render_row(p, row, y);
    }
}

} // namespace surface_renderer
