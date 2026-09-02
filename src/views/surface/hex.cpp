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
    options.address_label = "ADDRESS";

    hexview->setOptions(options);
}

} // namespace

HexViewDelegate::HexViewDelegate(QObject* parent): QHexDelegate(parent) {}

void HexViewDelegate::set_flags_buffer(const RDFlagsBuffer* flags) {
    m_flags = flags;
}

bool HexViewDelegate::renderByte(quint64 offset, quint8 b,
                                 QHexCharFormat& outcf,
                                 const QHexView* hexview) const {
    Q_UNUSED(b);
    Q_UNUSED(hexview);

    if(!m_flags) return false;

    auto idx = static_cast<usize>(offset);

    if(rd_flagsbuffer_has_patch(m_flags, idx))
        outcf.foreground = theme_provider::color(RD_THEME_FAIL);
    else
        return false;

    return true;
}

HexView::HexView(RDContext* ctx, QWidget* parent)
    : QWidget{parent}, m_context{ctx} {

    m_delegate = new HexViewDelegate(this);

    m_hexview = new QHexView{};
    m_hexview->setFont(surface_renderer::get_font());
    m_hexview->setDelegate(m_delegate);
    hexview_setup_options(m_hexview, ctx);

    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(m_hexview);

    this->create_popup_menu();

    connect(m_hexview, &QHexView::positionChanged, this,
            [&]() { statusbar::set_address(this); });
}

void HexView::clear_changes() { m_hexview->clearChanges(); }

void HexView::clear_history() {}
bool HexView::can_go_back() const { return false; }
bool HexView::can_go_forward() const { return false; }

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
        auto* flagsbuffer = new FlagsBuffer(m_context, seg);

        m_segment = seg;

        QHexDocument* hexdocument = QHexDocument::fromBuffer(flagsbuffer);
        m_delegate->set_flags_buffer(flagsbuffer->flags());

        m_hexview->setDocument(hexdocument);
        m_hexview->setBaseAddress(flagsbuffer->base_address());

        if(olddoc) olddoc->deleteLater();
    }

    m_hexview->hexCursor()->moveAddress(static_cast<quint64>(address));
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

    QAction* act_switch_listing =
        actions::create(actions::SWITCH_TO_LISTING, this);

    m_popupmenu->addAction("Copy Visual", this,
                           [&]() { m_hexview->copyVisual(); });

    m_popupmenu->addAction("Select All", QKeySequence{Qt::CTRL | Qt::Key_A},
                           this, [&]() { m_hexview->selectAll(); });

    m_popupmenu->addAction(hexview_new_separator(this));
    m_popupmenu->addAction(act_switch_listing);

    connect(this, &HexView::customContextMenuRequested, this,
            [=](const QPoint& pos) {
                act_copy->setVisible(m_hexview->hasSelection());
                m_popupmenu->exec(this->mapToGlobal(pos));
            });
}
