#include "node.h"
#include "support/surfacerenderer.h"
#include "support/themeprovider.h"
#include "support/utils.h"
#include <QApplication>
#include <QPainter>
#include <QWidget>

namespace {

constexpr int DROP_SHADOW_SIZE = 6;

}

SurfaceGraphNode::SurfaceGraphNode(RDSurfaceGraph* surface,
                                   const RDFunctionChunk* chunk, RDGraphNode n,
                                   QWidget* parent)
    : GraphViewNode{n, parent}, m_chunk{chunk}, m_surface{surface} {
    this->update_metrics();
}

bool SurfaceGraphNode::contains_address(RDAddress address) const {
    return address >= m_chunk->start && address < m_chunk->end;
}

int SurfaceGraphNode::current_row() const {
    RDAddress address;

    if(rd_surfacegraph_get_current_address(m_surface, &address) &&
       this->contains_address(address)) {
        RDSurfacePos pos = rd_surfacegraph_get_pos(m_surface);
        return pos.row - this->start_row();
    }

    return GraphViewNode::current_row();
}

QSize SurfaceGraphNode::size() const {
    int s = this->start_row();
    int e = this->end_row();
    if(s == -1 || e == -1) return {};

    return {
        qCeil(m_maxwidth * surface_renderer::cell_width()),
        qCeil((e - s + 1) * surface_renderer::cell_height()),
    };
}

void SurfaceGraphNode::mousedoubleclick_event(QMouseEvent*) {
    Q_EMIT follow_requested();
}

void SurfaceGraphNode::mousepress_event(QMouseEvent* e) {
    if(e->buttons() == Qt::LeftButton) {
        RDSurfacePos pos;
        this->get_surface_pos(e->position(), &pos);
        rd_surfacegraph_set_pos(m_surface, pos.row, pos.col);
        this->invalidate();
    }
    else
        GraphViewNode::mousepress_event(e);

    e->accept();
}

void SurfaceGraphNode::mousemove_event(QMouseEvent* e) {
    if(e->buttons() != Qt::LeftButton) return;

    RDSurfacePos pos;
    this->get_surface_pos(e->position(), &pos);
    rd_surfacegraph_select(m_surface, pos.row, pos.col);
    this->invalidate();
    e->accept();
}

void SurfaceGraphNode::update_metrics() {
    int s = this->start_row();
    int e = this->end_row();

    if(s == -1 || e == -1) {
        m_maxwidth = 0;
        return;
    }

    m_maxwidth = 0;
    for(int i = s; i <= e; i++) {
        RDRowSlice row =
            rd_surfacegraph_get_row(m_surface, static_cast<usize>(i));
        m_maxwidth = qMax(m_maxwidth, row.content_length);
    }
}

void SurfaceGraphNode::get_surface_pos(const QPointF& pt,
                                       RDSurfacePos* pos) const {
    pos->row =
        this->start_row() + qFloor(pt.y() / surface_renderer::cell_height());
    pos->col = qFloor(pt.x() / surface_renderer::cell_width());
}

int SurfaceGraphNode::start_row() const {
    return rd_surfacegraph_index_of(m_surface, m_chunk->start);
}

int SurfaceGraphNode::end_row() const {
    return rd_surfacegraph_last_index_of(m_surface, m_chunk->end - 1);
}

void SurfaceGraphNode::render(QPainter* painter, usize state) {
    QRect r{QPoint{}, this->size()};
    r.adjust(BLOCK_MARGINS);

    QColor shadow = painter->pen().color().darker();
    shadow.setAlphaF(0.2);

    painter->save();
    painter->translate(this->position());

    this->draw_shadow(painter, r, state & SurfaceGraphNode::SELECTED);

    painter->fillRect(r, qApp->palette().color(QPalette::Base));

    int s = this->start_row();
    int e = this->end_row();

    if(s != -1 && e != -1) {
        painter->save();
        painter->setClipRect(r);
        surface_renderer::render_block(painter, m_surface,
                                       static_cast<usize>(s),
                                       static_cast<usize>(e - s) + 1);
        painter->restore();
    }

    if(state & SurfaceGraphNode::SELECTED)
        painter->setPen(QPen{qApp->palette().color(QPalette::Highlight), 2.0});
    else
        painter->setPen(QPen{qApp->palette().color(QPalette::WindowText), 1.5});

    painter->drawRect(r);
    painter->restore();
}

void SurfaceGraphNode::draw_shadow(QPainter* painter, const QRect& r,
                                   bool selected) {
    const QColor SHADOW = theme_provider::is_dark_theme()
                              ? QColor(255, 255, 255, 30)
                              : QColor(0, 0, 0, 40);

    if(selected) { // Thicker shadow
        painter->fillRect(r.adjusted(DROP_SHADOW_SIZE, DROP_SHADOW_SIZE,
                                     DROP_SHADOW_SIZE + 2,
                                     DROP_SHADOW_SIZE + 2),
                          SHADOW);
    }
    else {
        painter->fillRect(r.adjusted(DROP_SHADOW_SIZE, DROP_SHADOW_SIZE,
                                     DROP_SHADOW_SIZE, DROP_SHADOW_SIZE),
                          SHADOW);
    }
}
