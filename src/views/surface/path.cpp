#include "path.h"
#include "support/surfacerenderer.h"
#include "support/themeprovider.h"
#include "support/utils.h"
#include "views/surface/listing.h"
#include <QPainter>
#include <QPainterPath>

SurfacePath::SurfacePath(SurfaceListing* surface, QWidget* parent)
    : QWidget{parent}, m_surfacelisting{surface} {
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    connect(surface, &SurfaceListing::render_completed, this,
            qOverload<>(&SurfacePath::update));
}

void SurfacePath::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    RDSurfacePathSlice path = rd_surface_get_path(m_surfacelisting->handle());
    if(rd_slice_is_empty(path)) return;

    QPainter painter{this};
    painter.setFont(surface_renderer::get_font());

    const int NROWS = m_surfacelisting->visible_rows();
    const qreal W = surface_renderer::cell_width();
    const qreal H = surface_renderer::cell_height();

    qreal x = this->width() - (W * 2);

    for(usize i = 0; i < rd_slice_length(path); i++, x -= W) {
        const RDSurfacePathItem* p = &rd_slice_at(path, i);

        int y1 = (p->from_row * H) + (H / 2);
        int y2 = (qMin(p->to_row, NROWS + 1) * H) + (H / 2);
        int y = (qMin(p->to_row, NROWS + 1) * H);

        QVector<QLine> points;
        points.push_back(QLine(this->width(), y1, x, y1));
        points.push_back(QLine(x, y1, x, y2));
        points.push_back(QLine(x, y2, this->width(), y2));

        qreal penwidth = this->is_path_selected(p) ? 3 : 2;

        if(y2 > (y + (H / 2)))
            y2 -= penwidth;
        else if(y2 < (y + (H / 2)))
            y2 += penwidth;

        Qt::PenStyle penstyle;

        if(!p->is_loop) {
            bool dotted = p->from_row == -1 || p->to_row == 1 ||
                          p->from_row >= NROWS || p->to_row >= NROWS;

            penstyle = dotted ? Qt::DotLine : Qt::SolidLine;
        }
        else
            penstyle = Qt::DashDotLine;

        painter.setPen(
            QPen{themeprovider::color(p->style), penwidth, penstyle});
        painter.drawLines(points);

        painter.setPen(
            QPen{themeprovider::color(p->style), penwidth, Qt::SolidLine});
        this->fill_arrow(&painter, y2);
    }
}

bool SurfacePath::is_path_selected(const RDSurfacePathItem* p) const {
    int row = m_surfacelisting->get_position().row;
    return (row == p->from_row) || (row == p->to_row);
}

void SurfacePath::fill_arrow(QPainter* painter, int y) {
    const int W = surface_renderer::cell_width() / 2;
    const int HL = surface_renderer::cell_height() / 3;

    QPainterPath path;
    path.moveTo(QPoint{this->width() - W, y});
    path.lineTo(QPoint{this->width() - W, y - HL});
    path.lineTo(QPoint{this->width(), y});
    path.lineTo(QPoint{this->width() - W, y + HL});
    path.lineTo(QPoint{this->width() - W, y});

    painter->fillPath(path, painter->pen().brush());
}
