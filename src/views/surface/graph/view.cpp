#include "view.h"
#include "statusbar.h"
#include "support/utils.h"

SurfaceGraphView::SurfaceGraphView(RDContext* ctx, QWidget* parent)
    : GraphView{parent}, m_context{ctx} {
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    m_surface = rd_surfacegraph_create(ctx, RD_RF_GRAPH);
    m_popup = new SurfacePopup(ctx, this);
    m_menu = utils::create_surface_menu(this);

    connect(this, &SurfaceGraphView::customContextMenuRequested, this,
            [&](const QPoint&) {
                if(this->selected_item()) m_menu->popup(QCursor::pos());
            });
}

SurfaceGraphView::~SurfaceGraphView() {
    rd_surfacegraph_destroy(m_surface);
    m_surface = nullptr;
}

std::optional<RDAddress> SurfaceGraphView::function_address() const {
    RDAddress a;
    if(rd_surfacegraph_get_function_address(m_surface, &a)) return a;
    return std::nullopt;
}

bool SurfaceGraphView::can_go_back() const {
    return rd_surfacegraph_can_go_back(m_surface);
}

bool SurfaceGraphView::can_go_forward() const {
    return rd_surfacegraph_can_go_forward(m_surface);
}

bool SurfaceGraphView::has_selection() const {
    return rd_surfacegraph_has_selection(m_surface);
}

RDRenderMode SurfaceGraphView::get_mode() const {
    return rd_surfacegraph_get_mode(m_surface);
}

RDGraph* SurfaceGraphView::graph() const {
    return reinterpret_cast<RDGraph*>(rd_surfacegraph_get_graph(m_surface));
}

void SurfaceGraphView::jump_to_ep() {
    RDAddress ep;
    if(!rd_get_entry_point(m_context, &ep)) return;

    auto prev = this->function_address();
    if(!rd_surfacegraph_jump_to(m_surface, ep)) return;

    this->track_function_change(prev);
    this->invalidate();
    this->clear_history();
}

void SurfaceGraphView::jump_to(RDAddress address) {
    auto prev = this->function_address();

    if(!rd_surfacegraph_jump_to(m_surface, address)) {
        // fallback to listing
        Q_EMIT view_requested(ViewRequest::LISTING, address);
        return;
    }

    this->track_function_change(prev);
    this->invalidate();
    Q_EMIT history_updated();
}

bool SurfaceGraphView::invalidate() {
    rd_surfacegraph_render(m_surface);
    this->update_graph();
    return true;
}

void SurfaceGraphView::refresh_content() {
    rd_surfacegraph_render(m_surface); //  may repopulate (new gen, new ids)

    if(rd_graph_get_gen(this->graph()) != m_lastgen)
        this->update_graph(); // rebuild nodes/edges/layout
    else
        this->viewport()->update(); // content only, don't relayout

    statusbar::set_address(this);
}

void SurfaceGraphView::set_mode(RDRenderMode m) {
    rd_surfacegraph_set_mode(m_surface, m);
    this->invalidate();
}

bool SurfaceGraphView::set_position(int row, int col) {
    if(rd_surfacegraph_set_pos(m_surface, row, col)) {
        this->invalidate();
        return true;
    }

    return false;
}

bool SurfaceGraphView::select(int row, int col) {
    if(rd_surfacegraph_select(m_surface, row, col)) {
        this->invalidate();
        return true;
    }

    return false;
}

bool SurfaceGraphView::select_all() {
    SurfaceGraphNode* n = this->find_node_at_cursor();
    if(!n) return false;

    rd_surfacegraph_set_pos(m_surface, n->start_row(), 0);
    rd_surfacegraph_select(m_surface, n->end_row(), INT_MAX);
    this->invalidate();
    return true;
}

bool SurfaceGraphView::go_back() {
    auto prev = this->function_address();
    if(!rd_surfacegraph_go_back(m_surface)) return false;

    this->track_function_change(prev);
    this->invalidate();
    Q_EMIT history_updated();
    return true;
}

bool SurfaceGraphView::go_forward() {
    auto prev = this->function_address();
    if(!rd_surfacegraph_go_forward(m_surface)) return false;

    this->track_function_change(prev);
    this->invalidate();
    Q_EMIT history_updated();
    return true;
}

void SurfaceGraphView::clear_history() {
    rd_surfacegraph_clear_history(m_surface);
    Q_EMIT history_updated();
}

void SurfaceGraphView::clear_selection() {
    rd_surfacegraph_clear_selection(m_surface);
    this->invalidate();
}

void SurfaceGraphView::begin_compute() {
    if(m_functionchanged) this->reset_zoom();
}

void SurfaceGraphView::end_compute() {
    statusbar::set_address(this);
    if(!m_functionchanged) return;

    if(SurfaceGraphNode* n = this->find_node_at_cursor(); n)
        this->focus_block(n);
    else
        this->focus_root_block();

    m_functionchanged = false;
}

void SurfaceGraphView::update_node(GraphViewNode* item) {
    auto* g = static_cast<SurfaceGraphNode*>(item);
    g->update_metrics();
}

GraphViewNode* SurfaceGraphView::create_node(RDGraphNode n, const RDGraph*) {
    auto* g = new SurfaceGraphNode(m_surface, n, this);

    connect(g, &SurfaceGraphNode::follow_requested, this, [&]() {
        RDAddress address;

        if(rd_surfacegraph_get_address_under_cursor(m_surface, &address))
            this->jump_to(address);
    });

    connect(g, &SurfaceGraphNode::invalidated, this,
            &SurfaceGraphView::refresh_content);

    return g;
}

void SurfaceGraphView::keyPressEvent(QKeyEvent* e) {
    if(!utils::handle_key_press(this, e)) {
        RDSurfacePos pos = this->get_position();
        auto [row, col] = pos;

        if(e->matches(QKeySequence::MoveToNextLine)) {
            this->set_position(row + 1, col);
        }
        else if(e->matches(QKeySequence::MoveToPreviousLine)) {
            if(row > 0) this->set_position(row - 1, col);
        }
        else
            GraphView::keyPressEvent(e);

        return;
    }

    this->invalidate();
}

void SurfaceGraphView::focusInEvent(QFocusEvent* e) {
    GraphView::focusInEvent(e);

    if(m_surface) {
        rd_surfacegraph_set_cursor_visible(m_surface, true);
        this->invalidate();
    }
}

void SurfaceGraphView::focusOutEvent(QFocusEvent* e) {
    GraphView::focusOutEvent(e);

    if(m_surface) {
        rd_surfacegraph_set_cursor_visible(m_surface, false);
        this->invalidate();
    }
}

bool SurfaceGraphView::event(QEvent* event) {
    if(m_surface && event->type() == QEvent::ToolTip) {
        auto* helpevent = static_cast<QHelpEvent*>(event);
        this->show_popup(helpevent->pos());
        return true;
    }

    return GraphView::event(event);
}

RDSurfacePos SurfaceGraphView::get_position() const {
    return rd_surfacegraph_get_pos(m_surface);
}

QString SurfaceGraphView::get_selected_text() const {
    return QString::fromUtf8(rd_surfacegraph_get_selected_text(m_surface));
}

std::optional<ISurfaceRange> SurfaceGraphView::get_selected_range() const {
    RDAddress start, end;
    if(!rd_surfacegraph_get_selected_range(m_surface, &start, &end))
        return std::nullopt;

    return ISurfaceRange{start, end};
}

std::optional<RDAddress> SurfaceGraphView::get_current_address() const {
    RDAddress address;
    if(rd_surfacegraph_get_current_address(m_surface, &address)) return address;
    return std::nullopt;
}

std::optional<RDAddress> SurfaceGraphView::get_address_under_cursor() const {
    RDAddress address;
    if(rd_surfacegraph_get_address_under_cursor(m_surface, &address))
        return address;
    return std::nullopt;
}

std::optional<RDCellData> SurfaceGraphView::get_cell_data_under_cursor() const {
    RDCellData cd;
    if(rd_surfacegraph_get_cell_data_under_cursor(m_surface, &cd)) return cd;
    return std::nullopt;
}

SurfaceGraphNode* SurfaceGraphView::find_node(RDAddress address) {
    for(GraphViewNode* g : m_nodes) {
        auto* sg = static_cast<SurfaceGraphNode*>(g);
        if(sg->contains_address(address)) return sg;
    }

    return nullptr;
}

SurfaceGraphNode* SurfaceGraphView::find_node_at_cursor() {
    auto address = this->get_current_address();
    if(!address) return nullptr;

    return this->find_node(*address);
}

void SurfaceGraphView::track_function_change(
    const std::optional<RDAddress>& prev) {
    auto curr = this->function_address();
    m_functionchanged = !prev || (curr && *prev != *curr);
}

void SurfaceGraphView::show_popup(const QPoint& pt) {
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
            const char* word =
                rd_surfacegraph_get_word_under_pos(m_surface, &pos);
            m_popup->popup(address, word ? QString::fromUtf8(word) : QString{});
        }
        else
            m_popup->hide();
    }
}
