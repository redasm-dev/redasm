#include "graph.h"
#include "statusbar.h"
#include "support/utils.h"

SurfaceGraph::SurfaceGraph(RDContext* ctx, QWidget* parent)
    : GraphView{parent}, m_context{ctx} {
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setCornerWidget(utils::create_screenshot_button(this->viewport()));

    m_surface = rd_surfacegraph_create(ctx, RD_RENDERER_GRAPH);
    m_popup = new SurfacePopup(ctx, this);
    m_menu = utils::create_surface_menu(this);

    connect(this, &SurfaceGraph::customContextMenuRequested, this,
            [&](const QPoint&) {
                if(this->selected_item()) m_menu->popup(QCursor::pos());
            });
}

SurfaceGraph::~SurfaceGraph() {
    rd_surfacegraph_destroy(m_surface);
    m_surface = nullptr;
}

bool SurfaceGraph::can_go_back() const {
    return rd_surfacegraph_can_go_back(m_surface);
}

bool SurfaceGraph::can_go_forward() const {
    return rd_surfacegraph_can_go_forward(m_surface);
}

bool SurfaceGraph::has_selection() const {
    return rd_surfacegraph_has_selection(m_surface);
}

bool SurfaceGraph::has_rdil() const {
    return rd_surfacegraph_has_rdil(m_surface);
}

void SurfaceGraph::jump_to_ep() {
    const RDFunction* prev = rd_surfacegraph_get_function(m_surface);

    if(rd_surfacegraph_jump_to_ep(m_surface)) {
        const RDFunction* curr = rd_surfacegraph_get_function(m_surface);
        m_functionchanged = (prev != curr);

        this->set_graph(rd_surfacegraph_get_graph(m_surface));
        Q_EMIT history_updated();
    }
}

void SurfaceGraph::jump_to(RDAddress address) {
    const RDFunction* prev = rd_surfacegraph_get_function(m_surface);

    if(rd_surfacegraph_jump_to(m_surface, address)) {
        const RDFunction* curr = rd_surfacegraph_get_function(m_surface);
        m_functionchanged = (prev != curr);

        this->set_graph(rd_surfacegraph_get_graph(m_surface));
        Q_EMIT history_updated();
    }
}

void SurfaceGraph::invalidate() {
    rd_surfacegraph_render(m_surface);
    this->update_graph();
}

void SurfaceGraph::set_rdil(bool b) {
    rd_surfacegraph_set_rdil(m_surface, b);
    this->invalidate();
}

void SurfaceGraph::set_position(int row, int col) {
    if(rd_surfacegraph_set_pos(m_surface, row, col)) this->invalidate();
}

void SurfaceGraph::select(int row, int col) {
    if(rd_surfacegraph_select(m_surface, row, col)) this->invalidate();
}

bool SurfaceGraph::go_back() {
    const RDFunction* prev = rd_surfacegraph_get_function(m_surface);

    if(rd_surfacegraph_go_back(m_surface)) {
        const RDFunction* curr = rd_surfacegraph_get_function(m_surface);
        m_functionchanged = (prev != curr);

        if(m_functionchanged)
            this->set_graph(rd_surfacegraph_get_graph(m_surface));
        else
            this->viewport()->update();

        Q_EMIT history_updated();
        return true;
    }

    return false;
}

bool SurfaceGraph::go_forward() {
    const RDFunction* prev = rd_surfacegraph_get_function(m_surface);

    if(rd_surfacegraph_go_forward(m_surface)) {
        const RDFunction* curr = rd_surfacegraph_get_function(m_surface);
        m_functionchanged = (prev != curr);

        if(m_functionchanged)
            this->set_graph(rd_surfacegraph_get_graph(m_surface));
        else
            this->viewport()->update();

        Q_EMIT history_updated();
        return true;
    }

    return false;
}

void SurfaceGraph::clear_history() {
    rd_surfacegraph_clear_history(m_surface);
    Q_EMIT history_updated();
}

void SurfaceGraph::compute_layout() {
    if(!m_surface) return;

    RDGraph* g = rd_surfacegraph_get_graph(m_surface);
    if(g) rd_graph_compute_layered(g, RD_LAYERED_LAYOUT_MEDIUM);
}

void SurfaceGraph::end_compute() {
    statusbar::set_address(this);
    if(!m_functionchanged) return;

    if(SurfaceGraphNode* n = this->find_node_at_cursor(); n)
        this->focus_block(n);
    else
        this->focus_root_block();

    m_functionchanged = false;
}

void SurfaceGraph::update_node(GraphViewNode* item) {
    auto* g = static_cast<SurfaceGraphNode*>(item);
    g->update_metrics();
}

GraphViewNode* SurfaceGraph::create_node(RDGraphNode n, const RDGraph*) {
    const RDFunction* f = rd_surfacegraph_get_function(m_surface);
    RDFunctionChunk b;

    if(rd_function_get_chunk(f, n, &b)) {
        auto* g = new SurfaceGraphNode(m_surface, b, n, this);
        g->setObjectName(QString::number(b.start, 16));

        connect(g, &SurfaceGraphNode::follow_requested, this, [&]() {
            RDAddress address;

            if(rd_surfacegraph_get_address_under_cursor(m_surface, &address))
                this->jump_to(address);
        });

        connect(g, &SurfaceGraphNode::invalidated, this,
                &SurfaceGraph::invalidate);
        return g;
    }

    return nullptr;
}

void SurfaceGraph::keyPressEvent(QKeyEvent* e) {
    if(!utils::handle_key_press(this, e)) {
        if(e->key() == Qt::Key_Space)
            Q_EMIT switch_view();
        else
            GraphView::keyPressEvent(e);

        return;
    }

    this->invalidate();
}

void SurfaceGraph::focusInEvent(QFocusEvent* e) {
    GraphView::focusInEvent(e);

    if(m_surface) {
        rd_surfacegraph_set_cursor_visible(m_surface, true);
        this->invalidate();
    }

    statusbar::check_rdil();
}

void SurfaceGraph::focusOutEvent(QFocusEvent* e) {
    GraphView::focusOutEvent(e);

    if(m_surface) {
        rd_surfacegraph_set_cursor_visible(m_surface, false);
        this->invalidate();
    }

    statusbar::check_rdil();
}

bool SurfaceGraph::event(QEvent* event) {
    if(m_surface && event->type() == QEvent::ToolTip) {
        auto* helpevent = static_cast<QHelpEvent*>(event);
        this->show_popup(helpevent->pos());
        return true;
    }

    return GraphView::event(event);
}

RDSurfacePos SurfaceGraph::get_position() const {
    return rd_surfacegraph_get_pos(m_surface);
}

QString SurfaceGraph::get_selected_text() const {
    return QString::fromUtf8(rd_surfacegraph_get_selected_text(m_surface));
}

std::optional<RDAddress> SurfaceGraph::get_current_address() const {
    RDAddress address;
    if(rd_surfacegraph_get_current_address(m_surface, &address)) return address;
    return std::nullopt;
}

std::optional<RDAddress> SurfaceGraph::get_address_under_cursor() const {
    RDAddress address;
    if(rd_surfacegraph_get_address_under_cursor(m_surface, &address))
        return address;
    return std::nullopt;
}

SurfaceGraphNode* SurfaceGraph::find_node(RDAddress address) {
    for(GraphViewNode* g : m_nodes) {
        auto* sg = static_cast<SurfaceGraphNode*>(g);
        if(sg->contains_address(address)) return sg;
    }

    return nullptr;
}

SurfaceGraphNode* SurfaceGraph::find_node_at_cursor() {
    auto address = this->get_current_address();
    if(!address) return nullptr;

    return this->find_node(*address);
}

void SurfaceGraph::show_popup(const QPoint& pt) {
    if(!m_surface) return;

    QPoint nodepos;
    const auto* n = static_cast<SurfaceGraphNode*>(
        this->node_from_pos(pt.toPointF(), &nodepos));

    if(n) {
        RDSurfacePos pos;
        n->get_surface_pos(nodepos.toPointF(), &pos);

        RDAddress address;
        if(rd_surfacegraph_get_address_under_pos(m_surface, &pos, &address) &&
           rd_surfacegraph_index_of(m_surface, address) == -1) {
            // m_popup->popup(address);
            return;
        }

        m_popup->hide();
    }
}
