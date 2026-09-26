#pragma once

#define RENDERER_BLOCK_CONTENT 4

#define RENDERER_BLOCK_CONTENTS                                                \
    -RENDERER_BLOCK_CONTENT, 0, -RENDERER_BLOCK_CONTENT, 0

#include <QFont>
#include <QPoint>
#include <redasm/redasm.h>

class QPainter;

namespace surface_renderer {

void init();
void render(QPainter* p, RDSurface* surface, usize n, usize start_x = 0);
void render_block(QPainter* p, RDSurfaceGraph* surface, usize start, usize n);
qreal cell_width();
qreal cell_height();
RDSurfacePos hit_test(QPoint pt, quint64 start_x);
QFont get_font();

} // namespace surface_renderer
