#include "node.h"
#include "support/surfacerenderer.h"
#include "support/utils.h"
#include <QApplication>
#include <QPainter>
#include <QWidget>

namespace {

constexpr int DROP_SHADOW_SIZE = 10;

}

SurfaceGraphNode::SurfaceGraphNode(RDSurfaceGraph* surface,
                                   const RDFunctionChunk& b, RDGraphNode n,
                                   QWidget* parent)
    : GraphViewNode{n, parent}, m_block{b}, m_surface{surface} {
    this->update_metrics();
}

bool SurfaceGraphNode::contains_address(RDAddress address) const {
    return address >= m_block.start && address < m_block.end;
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
        RDRowSlice row = rd_surfacegraph_get_row(m_surface, i);
        m_maxwidth = qMax(m_maxwidth, static_cast<int>(row.content_length));
    }
}

void SurfaceGraphNode::get_surface_pos(const QPointF& pt,
                                       RDSurfacePos* pos) const {
    pos->row =
        this->start_row() + qFloor(pt.y() / surface_renderer::cell_height());
    pos->col = qFloor(pt.x() / surface_renderer::cell_width());
}

int SurfaceGraphNode::start_row() const {
    return rd_surfacegraph_index_of(m_surface, m_block.start);
}

int SurfaceGraphNode::end_row() const {
    return rd_surfacegraph_last_index_of(m_surface, m_block.end - 1);
}

void SurfaceGraphNode::render(QPainter* painter, usize state) {
    QRect r{QPoint{}, this->size()};
    r.adjust(BLOCK_MARGINS);

    QColor shadow = painter->pen().color();
    shadow.setAlpha(127);

    painter->save();
    painter->translate(this->position());

    if(state & SurfaceGraphNode::SELECTED) // Thicker shadow
        painter->fillRect(r.adjusted(DROP_SHADOW_SIZE, DROP_SHADOW_SIZE,
                                     DROP_SHADOW_SIZE + 2,
                                     DROP_SHADOW_SIZE + 2),
                          shadow);
    else
        painter->fillRect(r.adjusted(DROP_SHADOW_SIZE, DROP_SHADOW_SIZE,
                                     DROP_SHADOW_SIZE, DROP_SHADOW_SIZE),
                          shadow);

    painter->fillRect(r, qApp->palette().color(QPalette::Base));

    int s = this->start_row();
    int e = this->end_row();

    if(s != -1 && e != -1) {
        painter->save();
        painter->setClipRect(r);
        surface_renderer::render_block(painter, m_surface, s, e - s + 1);
        painter->restore();
    }

    if(state & SurfaceGraphNode::SELECTED)
        painter->setPen(QPen{qApp->palette().color(QPalette::Highlight), 2.0});
    else
        painter->setPen(QPen{qApp->palette().color(QPalette::WindowText), 1.5});

    painter->drawRect(r);
    painter->restore();
}
