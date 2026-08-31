#include "hex.h"
#include "hexview/flagsbuffer.h"
#include "statusbar.h"
#include "support/actions.h"
#include "support/surfacerenderer.h"
#include "support/themeprovider.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QVBoxLayout>

namespace {

QAction* hexview_new_separator(QWidget* parent) {
    auto* act = new QAction{parent};
    act->setSeparator(true);
    return act;
}

void hexview_setup_options(QHexView* hexview, RDContext* ctx) {
    QHexOptions options = hexview->options();

    options.byte_colors[0x00].foreground =
        theme_provider::color(RD_THEME_MUTED);
    options.address_width = rd_get_ptr_size(ctx) * 2;
    options.address_label = "Address";

    hexview->setOptions(options);
}

} // namespace

HexView::HexView(RDContext* ctx, QWidget* parent)
    : QWidget{parent}, m_context{ctx} {

    m_hexview = new QHexView{};
    m_hexview->setReadOnly(true);
    m_hexview->setFont(surface_renderer::get_font());
    hexview_setup_options(m_hexview, ctx);

    auto* vbox = new QVBoxLayout(this);
    vbox->addWidget(m_hexview);

    this->create_popup_menu();

    connect(m_hexview, &QHexView::positionChanged, this,
            [&]() { statusbar::set_address(this); });
}

void HexView::clear_history() {}

void HexView::jump_to_ep() {
    RDAddress ep;
    if(!rd_get_entry_point(m_context, &ep)) return;
    this->jump_to(ep);
    this->clear_history();
}

void HexView::jump_to(RDAddress address) {
    const RDSegment* seg = rd_find_segment(m_context, address);
    if(!seg) return;

    if(m_segment != seg) {
        QHexDocument* olddoc = m_hexview->hexDocument();
        auto* flagsbuffer = new FlagsBuffer(seg);

        m_segment = seg;

        QHexDocument* hexdocument = QHexDocument::fromBuffer(flagsbuffer);
        m_hexview->setDocument(hexdocument);
        m_hexview->setBaseAddress(flagsbuffer->base_address());
        m_hexview->hexCursor()->moveAddress(static_cast<quint64>(address));

        if(olddoc) olddoc->deleteLater();
    }
}

void HexView::set_mode(RDRenderMode m) { RD_UNUSED(m); }

bool HexView::invalidate() {
    m_hexview->update();
    return true;
}

bool HexView::go_back() { return false; }

bool HexView::go_forward() { return false; }

// QHexView has its own moving algorithm
RDSurfacePos HexView::get_position() const { return {}; }

// QHexView has its own moving algorithm
bool HexView::set_position(int row, int col) {
    Q_UNUSED(row);
    Q_UNUSED(col);
    return false;
}

// QHexView has its own selection algorithm
bool HexView::select(int row, int col) {
    Q_UNUSED(row);
    Q_UNUSED(col);
    return false;
}

// QHexView has its own selection algorithm
bool HexView::select_all() { return false; }

RDRenderMode HexView::get_mode() const { return RD_RM_NORMAL; }

// QHexView has its own clipboard algorithm
bool HexView::has_selection() const { return false; }
QString HexView::get_selected_text() const { return {}; }

bool HexView::can_go_back() const { return false; }
bool HexView::can_go_forward() const { return false; }

std::optional<ISurfaceRange> HexView::get_selected_range() const {
    auto base = static_cast<RDAddress>(m_hexview->baseAddress());

    return ISurfaceRange{
        base + m_hexview->selectionStartOffset(),
        base + m_hexview->selectionEndOffset(),
    };
}

std::optional<RDAddress> HexView::get_current_address() const {
    return static_cast<RDAddress>(m_hexview->hexCursor()->address());
}

std::optional<RDAddress> HexView::get_address_under_cursor() const {
    return this->get_current_address();
}

std::optional<RDCellData> HexView::get_cell_data_under_cursor() const {
    return std::nullopt;
}

void HexView::create_popup_menu() {
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    m_popupmenu = new QMenu(this);

    QAction* act_copy =
        m_popupmenu->addAction("Copy", QKeySequence{Qt::CTRL | Qt::Key_C}, this,
                               [&]() { m_hexview->copy(true); });

    m_popupmenu->addAction("Copy Visual", this,
                           [&]() { m_hexview->copyVisual(); });

    m_popupmenu->addAction("Select All", QKeySequence{Qt::CTRL | Qt::Key_A},
                           this, [&]() { m_hexview->selectAll(); });

    m_popupmenu->addAction(hexview_new_separator(this));
    m_popupmenu->addAction(actions::get(actions::SWITCH_TO_LISTING));

    connect(this, &HexView::customContextMenuRequested, this,
            [=](const QPoint& pos) {
                act_copy->setVisible(m_hexview->hasSelection());
                m_popupmenu->exec(this->mapToGlobal(pos));
            });
}
