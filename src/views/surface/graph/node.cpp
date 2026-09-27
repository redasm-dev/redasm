#include "node.h"
#include "support/surfacerenderer.h"
#include <QApplication>
#include <QPainter>
#include <QWidget>

SurfaceGraphNode::SurfaceGraphNode(RDSurfaceGraph* surface,
                                   const RDFunctionChunk* chunk, RDGraphNode n,
                                   QWidget* parent)
    : GraphViewNode{n, parent}, m_chunk{chunk}, m_surface{surface} {
    this->update_metrics();
}

bool SurfaceGraphNode::contains_address(RDAddress address) const {
    return address >= rd_functionchunk_get_start(m_chunk) &&
           address < rd_functionchunk_get_end(m_chunk);
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
        this->adjusted_width(
            qCeil(m_maxwidth * surface_renderer::cell_width())),
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
    return rd_surfacegraph_index_of(m_surface,
                                    rd_functionchunk_get_start(m_chunk));
}

int SurfaceGraphNode::end_row() const {
    RDAddress end_address = rd_functionchunk_get_end(m_chunk);

    // -1, chunk end is exclusive.
    // we don't want the last subline of the next range
    return rd_surfacegraph_last_index_of(m_surface, end_address - 1);
}

void SurfaceGraphNode::render(QPainter* p, usize state) {
    this->draw_chrome(p, state, true);

    int s = this->start_row();
    int e = this->end_row();

    if(s != -1 && e != -1) {
        p->save();
        p->setClipRect(this->rect());
        p->translate(this->adjusted_x(this->x()), this->y());
        surface_renderer::render_block(p, m_surface, static_cast<usize>(s),
                                       static_cast<usize>(e - s) + 1);
        p->restore();
    }

    this->draw_edge(p, state);
}
