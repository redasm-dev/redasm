#include "graph.h"
#include "node.h"
#include "support/surfacerenderer.h"
#include <QTimer>

CallGraph::CallGraph(RDContext* ctx, RDAddress address, QWidget* parent)
    : GraphView{parent}, m_context{ctx} {
    this->setFont(surface_renderer::get_font());

    m_graph = rd_callgraph_create(ctx, address);

    QTimer::singleShot(0, this, [&] { this->update_graph(); });
}

CallGraph::~CallGraph() { rd_callgraph_destroy(m_graph); }

RDGraph* CallGraph::graph() const {
    return reinterpret_cast<RDGraph*>(m_graph);
}

GraphViewNode* CallGraph::create_node(RDGraphNode n, const RDGraph* g) {
    auto* cgn = new CallGraphNode(
        m_context, reinterpret_cast<const RDCallGraph*>(g), n, this);

    connect(cgn, &CallGraphNode::address_requested, this,
            &CallGraph::address_requested);

    return cgn;
}

void CallGraph::compute_layout() {
    rd_graph_compute_radial(reinterpret_cast<RDGraph*>(m_graph));
}
