#pragma once

#include <QFont>
#include <QPoint>
#include <redasm/redasm.h>

class QPainter;

namespace surface_renderer {

void init();
void render(QPainter* p, RDSurface* surface, usize start, usize n);
void render_block(QPainter* p, RDSurfaceGraph* surface, usize start, usize n);
qreal cell_width();
qreal cell_height();
RDSurfacePos hit_test(QPoint pt);
QFont get_font();

} // namespace surface_renderer
