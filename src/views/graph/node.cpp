#include "node.h"

GraphViewNode::GraphViewNode(RDGraphNode node, QObject* parent)
    : QObject{parent}, m_node{node} {}

RDGraphNode GraphViewNode::node() const { return m_node; }
int GraphViewNode::x() const { return this->position().x(); }
int GraphViewNode::y() const { return this->position().y(); }
int GraphViewNode::width() const { return this->size().width(); }
int GraphViewNode::height() const { return this->size().height(); }
QRect GraphViewNode::rect() const { return {m_pos, this->size()}; }

bool GraphViewNode::contains(const QPoint& p) const {
    return this->rect().contains(p);
}

const QPoint& GraphViewNode::position() const { return m_pos; }
void GraphViewNode::move(const QPoint& pos) { m_pos = pos; }
void GraphViewNode::itemselection_changed(bool selected) { Q_UNUSED(selected); }

QPoint GraphViewNode::map_to_item(const QPoint& p) const {
    return {p.x() - m_pos.x(), p.y() - m_pos.y()};
}

int GraphViewNode::current_row() const { return 0; }
void GraphViewNode::mousedoubleclick_event(QMouseEvent* e) { Q_UNUSED(e); }
void GraphViewNode::mousepress_event(QMouseEvent* e) { Q_UNUSED(e); }
void GraphViewNode::mousemove_event(QMouseEvent* e) { Q_UNUSED(e); }

void GraphViewNode::invalidate(bool notify) {
    if(notify) Q_EMIT invalidated();
}
