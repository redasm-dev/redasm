#include "view.h"
#include "support/themeprovider.h"
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>
#include <redasm/redasm.h>

GraphView::GraphView(QWidget* parent): QAbstractScrollArea(parent) {
    QPalette palette = this->palette();
    palette.setColor(QPalette::Base, themeprovider::color(RD_THEME_GRAPH_BG));

    this->horizontalScrollBar()->setSingleStep(this->fontMetrics().height());
    this->verticalScrollBar()->setSingleStep(this->fontMetrics().height());
    this->setFrameShape(QFrame::NoFrame);
    this->setAutoFillBackground(true);
    this->setPalette(palette);
}

void GraphView::set_graph(RDGraph* graph) {
    m_selecteditem = nullptr;
    m_scalefactor = m_scaleboost = 1.0;
    m_graph = graph;
    qDeleteAll(m_nodes);
    m_nodes.clear();
    this->update_graph();
}

void GraphView::set_selected_node(const GraphViewNode* item) {
    for(GraphViewNode* gvn : m_nodes) {
        if(gvn != item) continue;
        m_selecteditem = gvn;
        this->focus_selected_block();
        break;
    }
}

void GraphView::set_focus_on_selection(bool b) { m_focusonselection = b; }
GraphViewNode* GraphView::selected_item() const { return m_selecteditem; }
RDGraph* GraphView::graph() const { return m_graph; }

void GraphView::focus_selected_block() {
    if(m_selecteditem) this->focus_block(m_selecteditem);
}

void GraphView::focus_block(const GraphViewNode* item, bool force) {
    // Don't update the view for blocks that are already fully in view
    int xofs = this->horizontalScrollBar()->value();
    int yofs = this->verticalScrollBar()->value();
    QRect viewportrect = this->viewport()->rect();

    // before being shown for the first time viewport is kind of 98x28 so
    // setting the parent size almost fixes this problem
    if(!m_viewportready)
        viewportrect.setSize(this->parentWidget()->size() - QSize(20, 20));

    QPoint translation(m_renderoffset.x() - xofs, m_renderoffset.y() - yofs);

    // Adjust scaled viewport
    viewportrect.setWidth(viewportrect.width() / m_scalefactor);
    viewportrect.setHeight(viewportrect.height() / m_scalefactor);
    viewportrect.translate(-translation.x() / m_scalefactor,
                           -translation.y() / m_scalefactor);

    QFontMetrics fm = this->fontMetrics();

    QRect r(item->x() + fm.height(), item->y() + fm.height(),
            item->width() - (2 * fm.height()),
            item->height() - (2 * fm.height()));

    if(force || !viewportrect.contains(r)) {
        auto x = (item->x() + (item->width() / 2.0)) * m_scalefactor;

        auto y = (item->y() + (2 * fm.height()) +
                  (item->current_row() * fm.height())) *
                 m_scalefactor;

        this->horizontalScrollBar()->setValue(
            x + m_renderoffset.x() -
            (this->horizontalScrollBar()->pageStep() / 2.0));

        this->verticalScrollBar()->setValue(
            y + m_renderoffset.y() -
            (this->verticalScrollBar()->pageStep() / 2.0));
    }
}

void GraphView::mouseDoubleClickEvent(QMouseEvent* e) {
    QPoint itempos;
    bool updated = this->update_selected_item(e, &itempos);

    if(m_selecteditem && (e->buttons() == Qt::LeftButton)) {
        QMouseEvent iteme{e->type(),   itempos,      e->globalPosition(),
                          e->button(), e->buttons(), e->modifiers()};
        m_selecteditem->mousedoubleclick_event(&iteme);
    }

    if(updated) this->selected_item_changed_event();
    QAbstractScrollArea::mouseDoubleClickEvent(e);
}

void GraphView::mousePressEvent(QMouseEvent* e) {
    QPoint itempos;
    bool updated = this->update_selected_item(e, &itempos);

    if(m_selecteditem) {
        QMouseEvent iteme = {
            e->type(),   itempos,      e->globalPosition(),
            e->button(), e->buttons(), e->modifiers(),
        };

        m_selecteditem->mousepress_event(&iteme);
    }
    else if(e->button() == Qt::LeftButton) {
        m_scrollmode = true;
        m_scrollbase = e->pos();
        this->setCursor(Qt::ClosedHandCursor);
        this->viewport()->grabMouse();
    }

    this->viewport()->update();
    if(updated) this->selected_item_changed_event();
    QAbstractScrollArea::mousePressEvent(e);
}

void GraphView::mouseReleaseEvent(QMouseEvent* e) {
    this->viewport()->update();

    if(e->button() == Qt::LeftButton && m_scrollmode) {
        m_scrollmode = false;
        this->setCursor(Qt::ArrowCursor);
        this->viewport()->releaseMouse();
    }

    QAbstractScrollArea::mouseReleaseEvent(e);
}

void GraphView::mouseMoveEvent(QMouseEvent* e) {
    if(m_selecteditem) {
        QPoint itempos;
        GraphViewNode* item = this->node_from_pos(e->position(), &itempos);

        if(item == m_selecteditem) {
            QMouseEvent iteme = {
                e->type(),   itempos,      e->globalPosition(),
                e->button(), e->buttons(), e->modifiers(),
            };

            m_selecteditem->mousemove_event(&iteme);
        }

        return;
    }

    if(m_scrollmode) {
        QPoint delta = {
            m_scrollbase.x() - qRound(e->position().x()),
            m_scrollbase.y() - qRound(e->position().y()),
        };

        m_scrollbase = e->pos();

        this->horizontalScrollBar()->setValue(
            this->horizontalScrollBar()->value() + delta.x());

        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() +
                                            delta.y());
    }

    QAbstractScrollArea::mouseMoveEvent(e);
}

void GraphView::wheelEvent(QWheelEvent* event) {
    if(event->modifiers() & Qt::ControlModifier) {
        m_scaleboost = event->modifiers() & Qt::ShiftModifier ? 2 : 1;

        QPoint ndegrees = event->angleDelta() / 8;
        QPoint nsteps = ndegrees / 15;

        if(nsteps.y() > 0) {
            m_scaledirection = 1;
            this->zoom_in(event->position());
        }
        else if(nsteps.y() < 0) {
            m_scaledirection = -1;
            this->zoom_out(event->position());
        }

        event->accept();
        return;
    }

    QAbstractScrollArea::wheelEvent(event);
}

void GraphView::resizeEvent(QResizeEvent* e) {
    this->adjust_size(e->size().width(), e->size().height());
}

void GraphView::paintEvent(QPaintEvent*) {
    QPoint translation = {
        m_renderoffset.x() - this->horizontalScrollBar()->value(),
        m_renderoffset.y() - this->verticalScrollBar()->value(),
    };

    QPainter painter(this->viewport());
    painter.translate(translation);
    painter.scale(m_scalefactor, m_scalefactor);

    QRect vpr = {
        this->viewport()->rect().topLeft(),
        this->viewport()->rect().bottomRight() - QPoint{1, 1},
    };

    // Adjust imaginary viewport rect at new zoom level
    vpr.setWidth(vpr.width() / m_scalefactor);
    vpr.setHeight(vpr.height() / m_scalefactor);
    vpr.translate(-translation.x() / m_scalefactor,
                  -translation.y() / m_scalefactor);

    // Render edges
    painter.save();

    for(auto& l : m_lines) {
        QColor c(rd_graph_get_edge_color(m_graph, std::addressof(l.first)));
        QPen pen(c);

        if(m_selecteditem && ((l.first.src == m_selecteditem->node()) ||
                              (l.first.dst == m_selecteditem->node())))
            pen.setWidthF(2.0 / m_scalefactor);
        else {
            pen.setWidthF(1.0 / m_scalefactor);
            pen.setStyle(m_selecteditem ? Qt::DashLine : Qt::SolidLine);
        }

        painter.setPen(pen);
        painter.setBrush(c);
        painter.drawLines(l.second);

        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.drawConvexPolygon(m_arrows[l.first]);
    }

    painter.restore();

    // Render nodes
    for(auto* item : m_nodes) {
        if(!vpr.intersects(item->rect())) // Ignore blocks that are not in view
            continue;

        usize itemstate = GraphViewNode::NONE;
        if(m_selecteditem == item) itemstate |= GraphViewNode::SELECTED;

        item->render(&painter, itemstate);
    }
}

void GraphView::showEvent(QShowEvent* e) {
    if(!m_viewportready) m_viewportready = true;
    e->ignore();
}

void GraphView::selected_item_changed_event() {
    if(m_focusonselection) this->focus_selected_block();
    Q_EMIT selected_item_changed();
}

void GraphView::update_edge(const RDGraphEdge&) {}
void GraphView::update_node(GraphViewNode*) {}
void GraphView::begin_compute() {}
void GraphView::end_compute() {}

void GraphView::compute_layout() {
    rd_graph_compute_layered(m_graph, RD_LAYERED_LAYOUT_MEDIUM);
}

void GraphView::focus_root_block() {
    if(!m_graph) return;

    auto it = m_nodes.find(rd_graph_get_root(m_graph));
    if(it != m_nodes.end()) this->focus_block(it.value());
}

void GraphView::update_graph() {
    m_lines.clear();
    m_arrows.clear();

    if(!m_graph) {
        this->viewport()->update();
        return;
    }

    rd_graph_clear_layout(m_graph);
    this->begin_compute();

    RDNodeSlice nodes = rd_graph_get_nodes(m_graph);
    QSet<RDGraphNode> nids;

    const RDGraphNode* n;
    rd_slice_each(n, nodes) {
        GraphViewNode* item = nullptr;

        if(!m_nodes.contains(*n)) {
            item = this->create_node(*n, m_graph);
            if(!item) continue;

            connect(item, &GraphViewNode::invalidated, this->viewport(),
                    [&]() { this->viewport()->update(); });

            item->setParent(this); // Take Ownership
            m_nodes[*n] = item;
        }
        else
            item = m_nodes[*n];

        this->update_node(item);
        nids.insert(*n);
        rd_graph_set_node_width(m_graph, item->node(), item->width());
        rd_graph_set_node_height(m_graph, item->node(), item->height());
    }

    // Remove orphan nodes
    for(auto it = m_nodes.begin(); it != m_nodes.end();) {
        if(!nids.contains(it.key())) {
            it.value()->deleteLater();
            it = m_nodes.erase(it);
        }
        else
            it++;
    }

    RDEdgeSlice edges = rd_graph_get_edges(m_graph);

    const RDGraphEdge* e;
    rd_slice_each(e, edges) { this->update_edge(*e); }

    this->compute_layout();

    rd_slice_each(n, nodes) {
        m_nodes[*n]->move(QPoint(rd_graph_get_node_x(m_graph, *n),
                                 rd_graph_get_node_y(m_graph, *n)));
    }

    rd_slice_each(e, edges) {
        this->precompute_line(*e);
        this->precompute_arrow(*e);
    }

    QSize areasize;
    if(m_viewportready)
        areasize = this->viewport()->size();
    else
        areasize = this->parentWidget()->size() - QSize(20, 20);

    qreal sx = static_cast<qreal>(areasize.width()) /
               static_cast<qreal>(this->width());
    qreal sy = static_cast<qreal>(areasize.height()) /
               static_cast<qreal>(this->height());
    m_scalemin = qMin(static_cast<qreal>(qMin(sx, sy) * (1 - m_scalestep)),
                      0.05); // If graph is very large...

    this->adjust_size(areasize.width(), areasize.height());
    this->viewport()->update();
    this->end_compute();
}

GraphViewNode* GraphView::node_from_pos(const QPointF& pt,
                                        QPoint* itempos) const {
    // Convert coordinates to system used in blocks
    int xofs = this->horizontalScrollBar()->value();
    int yofs = this->verticalScrollBar()->value();

    QPoint pos = {
        static_cast<int>(
            std::floor((pt.x() + xofs - m_renderoffset.x()) / m_scalefactor)),
        static_cast<int>(
            std::floor((pt.y() + yofs - m_renderoffset.y()) / m_scalefactor)),
    };

    for(GraphViewNode* item : m_nodes) {
        if(!item->contains(pos)) continue;
        if(itempos) *itempos = item->map_to_item(pos);
        return item;
    }

    return nullptr;
}

void GraphView::zoom_out(const QPointF& cursorpos) {
    m_prevscalefactor = m_scalefactor;

    if(m_scalefactor <= m_scalemin) return;
    m_scalefactor *= (1 - (m_scalestep * m_scaleboost));
    m_scalefactor = qMax(m_scalefactor, m_scalemin);

    QSize vpsize = this->viewport()->size();
    this->adjust_size(vpsize.width(), vpsize.height(), cursorpos);
    this->viewport()->update();
}

void GraphView::zoom_in(const QPointF& cursorpos) {
    m_prevscalefactor = m_scalefactor;

    if(m_scalefactor >= m_scalemax) return;

    m_scalefactor /= (1 - (m_scalestep * m_scaleboost));
    m_scalefactor = qMin(m_scalefactor, m_scalemax);

    QSize vpsize = this->viewport()->size();
    this->adjust_size(vpsize.width(), vpsize.height(), cursorpos);
    this->viewport()->update();
}

void GraphView::adjust_size(int vpw, int vph, const QPointF& cursorpos,
                            bool fit) {
    // bugfix - resize event (during several initial calls) may reset correct
    // adjustment already made
    if(!m_graph || (vph < 30)) return;

    m_rendersize = QSize{
        static_cast<int>(rd_graph_get_area_width(m_graph) * m_scalefactor),
        static_cast<int>(rd_graph_get_area_height(m_graph) * m_scalefactor),
    };

    m_renderoffset = QPoint(vpw, vph);

    QSize scrollrange = {
        m_rendersize.width() + vpw,
        m_rendersize.height() + vph,
    };

    qreal scalestepreal = 0.0;

    if(m_scaledirection > 0) {
        scalestepreal = (m_scalefactor - m_prevscalefactor) / m_scalefactor;
        scalestepreal /= (1 - scalestepreal);
    }
    else
        scalestepreal = (m_prevscalefactor - m_scalefactor) / m_prevscalefactor;

    QPoint deltaoffset{
        static_cast<int>(m_renderoffset.x() * scalestepreal * m_scaledirection),
        static_cast<int>(m_renderoffset.y() * scalestepreal * m_scaledirection),
    };

    QPoint oldscrollpos{
        this->horizontalScrollBar()->value(),
        this->verticalScrollBar()->value(),
    };

    this->horizontalScrollBar()->setPageStep(vpw);
    this->horizontalScrollBar()->setRange(0, scrollrange.width());
    this->verticalScrollBar()->setPageStep(vph);
    this->verticalScrollBar()->setRange(0, scrollrange.height());

    if(!cursorpos.isNull()) {
        QPointF deltacursorreal =
            cursorpos / m_prevscalefactor + oldscrollpos / m_prevscalefactor;
        QPointF deltacursordiff = deltacursorreal * m_scalefactor -
                                  deltacursorreal * m_prevscalefactor;

        this->horizontalScrollBar()->setValue(
            qRound(oldscrollpos.x() + deltacursordiff.x() - deltaoffset.x()));
        this->verticalScrollBar()->setValue(
            qRound(oldscrollpos.y() + deltacursordiff.y() - deltaoffset.y()));
    }
    else if(fit) {
        this->horizontalScrollBar()->setValue(scrollrange.width() / 2);
        this->verticalScrollBar()->setValue(scrollrange.height() / 2);
    }
}

void GraphView::precompute_arrow(const RDGraphEdge& e) {
    RDGraphPointSlice path = rd_graph_get_edge_arrow(m_graph, &e);
    QPolygon arrowhead;

    const RDGraphPoint* p;
    rd_slice_each(p, path) { arrowhead << QPoint{p->x, p->y}; }

    m_arrows[e] = arrowhead;
}

void GraphView::precompute_line(const RDGraphEdge& e) {
    RDGraphPointSlice path = rd_graph_get_edge_routes(m_graph, &e);
    usize c = rd_slice_length(path);
    QVector<QLine> lines;

    for(usize i = 0; c && (i < c - 1); i++) {
        const RDGraphPoint& p1 = rd_slice_at(path, i);
        const RDGraphPoint& p2 = rd_slice_at(path, i + 1);
        lines.push_back(QLine{p1.x, p1.y, p2.x, p2.y});
    }

    m_lines[e] = lines;
}

bool GraphView::update_selected_item(QMouseEvent* e, QPoint* itempos) {
    GraphViewNode* olditem = m_selecteditem;
    m_selecteditem = this->node_from_pos(e->position(), itempos);

    if(olditem) {
        olditem->itemselection_changed(false);
        olditem->invalidate();
    }

    if(m_selecteditem) {
        m_selecteditem->itemselection_changed(true);
        m_selecteditem->invalidate();
    }

    return olditem != m_selecteditem;
}
