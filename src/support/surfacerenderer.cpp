#include "surfacerenderer.h"
#include "support/settings.h"
#include "support/themeprovider.h"
#include <QPainter>

namespace surface_renderer {

namespace {

QFont g_font;
qreal g_cell_w = 0;
qreal g_cell_h = 0;
qreal g_baseline = 0;

void render_cells(QPainter* p, RDRowSlice row, usize start_x, qreal y) {
    const usize LEN = rd_slice_length(row);
    if(start_x >= LEN) return;

    const qreal BASELINE = y + g_baseline;

    QString text;
    usize runstart = 0;
    RDThemeKind runfg{}, runbg{};
    bool inrun = false;

    auto flush = [&]() {
        if(text.isEmpty()) return;

        QColor fg = theme_provider::color(runfg);
        QColor bg = theme_provider::color(runbg);
        Q_ASSERT(fg.isValid());
        Q_ASSERT(bg.isValid());

        const qreal X = static_cast<qreal>(runstart) * g_cell_w;

        p->fillRect(QRectF(X, y, text.size() * g_cell_w, g_cell_h), bg);
        p->setPen(fg);
        p->drawText(QPointF(X, BASELINE), text);

        text.clear();
    };

    usize col = 0;

    for(usize cx = start_x; cx < LEN; cx++, col++) {
        RDCell c = rd_slice_at(row, cx);

        if(!inrun || c.fg != runfg || c.bg != runbg) {
            flush();
            inrun = true;
            runfg = c.fg;
            runbg = c.bg;
            runstart = col;
        }

        text.append(QChar{static_cast<char32_t>(c.cp)});
    }

    flush();
}

} // namespace

void init() {
    g_font = REDasmSettings::load_font();

    QFontMetricsF fm{g_font};
    g_cell_w = fm.horizontalAdvance(" ");
    g_cell_h = fm.height();

    const qreal LEADING = g_cell_h - (fm.ascent() + fm.descent());
    g_baseline = (LEADING / 2.0) + fm.ascent();
}

RDSurfacePos hit_test(QPoint pt, quint64 start_x) {
    pt.rx() += surface_renderer::cell_width() * start_x;

    return {
        qFloor(pt.y() / g_cell_h),
        qFloor(pt.x() / g_cell_w),
    };
}

qreal cell_width() { return surface_renderer::g_cell_w; }
qreal cell_height() { return surface_renderer::g_cell_h; }
QFont get_font() { return surface_renderer::g_font; }

void render(QPainter* p, RDSurface* surface, usize n, usize start_x) {
    p->setFont(surface_renderer::g_font);

    for(usize i = 0; i < n; i++) {
        render_cells(p, rd_surface_get_row(surface, i), start_x,
                     static_cast<qreal>(i) * g_cell_h);
    }
}

void render_block(QPainter* p, RDSurfaceGraph* surface, usize start, usize n) {
    p->setFont(g_font);

    for(usize i = 0; i < n; i++) {
        render_cells(p, rd_surfacegraph_get_row(surface, start + i), 0,
                     static_cast<qreal>(i) * g_cell_h);
    }
}

} // namespace surface_renderer
