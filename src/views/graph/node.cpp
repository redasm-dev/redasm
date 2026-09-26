#include "node.h"
#include "support/surfacerenderer.h"
#include "support/themeprovider.h"
#include <QApplication>
#include <QPalette>

namespace {

constexpr int DROP_SHADOW_SIZE = 6;

}

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

QPoint GraphViewNode::position() const { return m_pos; }
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

void GraphViewNode::draw_shadow(QPainter* p, usize state) const {
    const QColor SHADOW = theme_provider::is_dark_theme()
                              ? QColor(255, 255, 255, 30)
                              : QColor(0, 0, 0, 40);

    if(state & GraphViewNode::SELECTED) { // thicker shadow on selection
        p->fillRect(this->rect().adjusted(DROP_SHADOW_SIZE, DROP_SHADOW_SIZE,
                                          DROP_SHADOW_SIZE + 2,
                                          DROP_SHADOW_SIZE + 2),
                    SHADOW);
    }
    else {
        p->fillRect(this->rect().adjusted(DROP_SHADOW_SIZE, DROP_SHADOW_SIZE,
                                          DROP_SHADOW_SIZE, DROP_SHADOW_SIZE),
                    SHADOW);
    }
}

int GraphViewNode::adjusted_x(int x) const {
    return x + RENDERER_BLOCK_CONTENT;
}

int GraphViewNode::adjusted_y(int y) const {
    return y + RENDERER_BLOCK_CONTENT;
}

int GraphViewNode::adjusted_width(int w) const {
    return w + (RENDERER_BLOCK_CONTENT * 2);
}

int GraphViewNode::adjusted_height(int h) const {
    return h + (RENDERER_BLOCK_CONTENT * 2);
}

void GraphViewNode::draw_chrome(QPainter* p, usize state) const {
    this->draw_shadow(p, state);
    p->fillRect(this->rect(), qApp->palette().color(QPalette::Base));
}

void GraphViewNode::draw_edge(QPainter* p, usize state) const {
    if(state & GraphViewNode::SELECTED)
        p->setPen(QPen{qApp->palette().color(QPalette::Highlight), 2.0});
    else
        p->setPen(QPen{qApp->palette().color(QPalette::WindowText), 1.5});

    p->drawRect(this->rect());
}
