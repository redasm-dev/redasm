#include "node.h"
#include "support/surfacerenderer.h"
#include "support/themeprovider.h"
#include "support/utils.h"

CallGraphNode::CallGraphNode(RDContext* ctx, const RDCallGraph* g,
                             RDGraphNode node, QWidget* parent)
    : GraphViewNode{node, parent}, m_context{ctx}, m_graph{g} {}

QSize CallGraphNode::size() const {
    QString s = this->get_node_text();

    return {
        this->adjusted_width(qCeil(s.size() * surface_renderer::cell_width())),
        qCeil(surface_renderer::cell_height()),
    };
}

void CallGraphNode::render(QPainter* p, usize state) {
    this->draw_chrome(p, state, false);

    QRect r = this->rect();
    r.setX(this->adjusted_x(r.x()));

    RDGraphNode root =
        rd_graph_get_root(reinterpret_cast<const RDGraph*>(m_graph));

    if(this->node() == root) {
        p->fillRect(this->rect(), theme_provider::color(RD_THEME_FOREGROUND));
        p->setPen(QPen{theme_provider::color(RD_THEME_BACKGROUND)});
    }

    p->drawText(r, this->get_node_text());

    this->draw_edge(p, state);
}

QString CallGraphNode::get_node_text() const {
    RDAddress address = rd_callgraph_get_address(m_graph, this->node());
    const char* name = rd_get_name(m_context, address);

    return name ? QString::fromUtf8(name) : utils::to_hex(address, m_context);
}

void CallGraphNode::mousedoubleclick_event(QMouseEvent* e) {
    RD_UNUSED(e);

    RDAddress address = rd_callgraph_get_address(m_graph, this->node());
    Q_EMIT address_requested(address);
}
