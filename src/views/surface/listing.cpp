#include "listing.h"
#include "statusbar.h"
#include "support/surfacerenderer.h"
#include "support/utils.h"
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

SurfaceListing::SurfaceListing(RDContext* ctx, QWidget* parent)
    : QAbstractScrollArea{parent}, m_context{ctx} {
    m_surface = rd_surface_create(ctx, RD_RENDERER_DEFAULT);
    m_popup = new SurfacePopup(ctx, this);

    this->setCornerWidget(utils::create_screenshot_button(this->viewport()));
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setCursor(Qt::ArrowCursor);
    this->setFrameShape(QFrame::NoFrame);
    this->setPalette(qApp->palette());
    this->verticalScrollBar()->setMinimum(0);
    this->verticalScrollBar()->setValue(0);
    this->verticalScrollBar()->setSingleStep(1);
    this->verticalScrollBar()->setPageStep(1);
    this->horizontalScrollBar()->setSingleStep(1);
    this->horizontalScrollBar()->setMinimum(0);
    this->horizontalScrollBar()->setValue(0);

    m_menu = utils::create_surface_menu(this);

    connect(this, &SurfaceListing::customContextMenuRequested, this,
            [&](const QPoint&) { m_menu->popup(QCursor::pos()); });

    connect(this->verticalScrollBar(), &QScrollBar::actionTriggered, this,
            [&](int) { rd_surface_clear_selection(m_surface); });

    this->update_scrollbars();
    this->sync_location();
}

SurfaceListing::~SurfaceListing() {
    rd_surface_destroy(m_surface);
    m_surface = nullptr;
}

bool SurfaceListing::has_selection() const {
    return rd_surface_has_selection(m_surface);
}

bool SurfaceListing::has_rdil() const { return rd_surface_has_rdil(m_surface); }

void SurfaceListing::set_rdil(bool v) {
    rd_surface_set_rdil(m_surface, v);
    this->viewport()->update();
}

void SurfaceListing::set_position(int row, int col) {
    rd_surface_set_pos(m_surface, row, col);
}

void SurfaceListing::select(int row, int col) {
    rd_surface_select(m_surface, row, col);
}

bool SurfaceListing::go_back() {
    if(rd_surface_go_back(m_surface)) {
        this->sync_location();
        Q_EMIT history_updated();
        return true;
    }

    return false;
}
bool SurfaceListing::go_forward() {
    if(rd_surface_go_forward(m_surface)) {
        this->sync_location();
        Q_EMIT history_updated();
        return true;
    }

    return false;
}

void SurfaceListing::clear_history() {
    rd_surface_clear_history(m_surface);
    Q_EMIT history_updated();
}

void SurfaceListing::jump_to(RDAddress address) {
    if(!rd_surface_jump_to(m_surface, address)) return;
    this->sync_location();
    Q_EMIT history_updated();
}

void SurfaceListing::jump_to_ep() {
    rd_surface_jump_to_ep(m_surface);
    Q_EMIT history_updated();
    this->sync_location();
}

void SurfaceListing::invalidate() {
    this->update_scrollbars();
    this->viewport()->update();
}

void SurfaceListing::mouseDoubleClickEvent(QMouseEvent* e) {
    if(e->button() == Qt::LeftButton) {
        if(!this->follow_under_cursor()) {
            RDSurfacePos p = this->get_surface_coords(e->pos());
            if(rd_surface_select_word(m_surface, p.row, p.col))
                this->viewport()->update();
        }
    }

    QAbstractScrollArea::mouseDoubleClickEvent(e);
}

void SurfaceListing::mousePressEvent(QMouseEvent* e) {
    if(m_surface) {
        switch(e->button()) {
            case Qt::LeftButton: {
                RDSurfacePos p = this->get_surface_coords(e->pos());
                rd_surface_set_pos(m_surface, p.row, p.col);
                Q_EMIT history_updated();
                this->viewport()->update();
                break;
            }

            case Qt::BackButton: this->go_back(); break;
            case Qt::ForwardButton: this->go_forward(); break;
            default: return;
        }

        e->accept();
    }

    QAbstractScrollArea::mousePressEvent(e);
}

void SurfaceListing::mouseMoveEvent(QMouseEvent* e) {
    if(m_surface && (e->buttons() == Qt::LeftButton)) {
        RDSurfacePos p = this->get_surface_coords(e->pos());
        if(rd_surface_select(m_surface, p.row, p.col))
            this->viewport()->update();
        e->accept();
        return;
    }

    QAbstractScrollArea::mouseMoveEvent(e);
}

void SurfaceListing::resizeEvent(QResizeEvent* e) {
    this->update_scrollbars();
    QAbstractScrollArea::resizeEvent(e);

    rd_surface_set_columns(m_surface, this->visible_columns());
    this->viewport()->update();
}

void SurfaceListing::keyPressEvent(QKeyEvent* e) {
    if(!utils::handle_key_press(this, e)) {
        QScrollBar* vscroll = this->verticalScrollBar();
        auto [row, col] = this->get_position();

        if(e->matches(QKeySequence::MoveToEndOfLine)) {
            rd_surface_set_pos(m_surface, row, this->visible_columns());
        }
        else if(e->matches(QKeySequence::MoveToStartOfDocument)) {
            vscroll->setValue(0);
            return;
        }
        else if(e->matches(QKeySequence::MoveToEndOfDocument)) {
            vscroll->setValue(vscroll->value() + this->visible_rows() - 1);
            return;
        }
        else if(e->matches(QKeySequence::SelectEndOfLine)) {
            rd_surface_select(m_surface, row, this->visible_columns());
        }
        else if(e->matches(QKeySequence::SelectEndOfDocument)) {
            rd_surface_select(m_surface, this->visible_rows() - row,
                              this->visible_columns());
        }
        else if(e->matches(QKeySequence::SelectAll)) {
            rd_surface_set_pos(m_surface, 0, 0);
            rd_surface_select(m_surface, this->visible_rows(),
                              this->visible_columns());
        }
        else if(e->key() == Qt::Key_Space) {
            Q_EMIT switch_view();
            return;
        }
        else {
            QAbstractScrollArea::keyPressEvent(e);
            return;
        }
    }

    this->viewport()->update();
}

void SurfaceListing::focusInEvent(QFocusEvent* e) {
    QAbstractScrollArea::focusInEvent(e);

    if(m_surface) {
        rd_surface_set_cursor_visible(m_surface, true);
        this->invalidate();
    }

    statusbar::check_rdil();
}

void SurfaceListing::focusOutEvent(QFocusEvent* e) {
    QAbstractScrollArea::focusInEvent(e);

    if(m_surface) {
        rd_surface_set_cursor_visible(m_surface, false);
        this->invalidate();
    }
}

void SurfaceListing::paintEvent(QPaintEvent* e) {
    if(!m_surface) {
        QAbstractScrollArea::paintEvent(e);
        return;
    }

    rd_surface_seek(m_surface, this->verticalScrollBar()->value());
    rd_surface_render(m_surface, this->visible_rows());

    QPainter p{this->viewport()};
    surface_renderer::render(&p, m_surface, 0,
                             rd_surface_get_row_count(m_surface));

    Q_EMIT render_completed();
    statusbar::set_address(this);
}

bool SurfaceListing::event(QEvent* event) {
    if(m_surface && event->type() == QEvent::ToolTip) {
        auto* helpevent = static_cast<QHelpEvent*>(event);
        this->show_popup(helpevent->pos());
        return true;
    }

    return QAbstractScrollArea::event(event);
}

bool SurfaceListing::can_go_back() const {
    return rd_surface_can_go_back(m_surface);
}

bool SurfaceListing::can_go_forward() const {
    return rd_surface_can_go_forward(m_surface);
}

int SurfaceListing::visible_columns() const {
    return qFloor(this->viewport()->width() / surface_renderer::cell_width());
}

int SurfaceListing::visible_rows() const {
    return qCeil(this->viewport()->height() / surface_renderer::cell_height());
}

RDSurfacePos SurfaceListing::get_surface_coords(QPoint pt) const {
    pt.ry() += this->horizontalScrollBar()->value();
    return surface_renderer::hit_test(pt);
}

RDSurfacePos SurfaceListing::get_position() const {
    return rd_surface_get_pos(m_surface);
}

std::optional<RDAddress> SurfaceListing::get_current_address() const {
    RDAddress address;
    if(rd_surface_get_current_address(m_surface, &address)) return address;
    return std::nullopt;
}

std::optional<RDAddress> SurfaceListing::get_address_under_cursor() const {
    RDAddress address;
    if(rd_surface_get_address_under_cursor(m_surface, &address)) return address;
    return std::nullopt;
}

QString SurfaceListing::get_selected_text() const {
    return QString::fromUtf8(rd_surface_get_selected_text(m_surface));
}

usize SurfaceListing::get_length() const {
    return rd_surface_get_length(m_surface);
}

bool SurfaceListing::follow_under_cursor() {
    RDAddress address;

    if(rd_surface_get_address_under_cursor(m_surface, &address)) {
        this->jump_to(address);
        return true;
    }

    return false;
}

void SurfaceListing::update_scrollbars() {
    this->verticalScrollBar()->setRange(0, this->get_length());
}

void SurfaceListing::sync_location() {
    usize currindex = rd_surface_get_row_start(m_surface);
    auto oldvalue = static_cast<usize>(this->verticalScrollBar()->value());

    if(oldvalue != currindex)
        this->verticalScrollBar()->setValue(currindex);
    else
        this->viewport()->update();
}

void SurfaceListing::show_popup(const QPoint& pt) {
    if(!m_surface) return;

    RDSurfacePos pos = this->get_surface_coords(pt);
    RDAddress address;

    if(rd_surface_get_address_under_pos(m_surface, &pos, &address) &&
       rd_surface_index_of(m_surface, address) == -1) {
        const char* word = rd_surface_get_word_under_pos(m_surface, &pos);
        m_popup->popup(address, word ? QString::fromUtf8(word) : QString{});
    }
    else
        m_popup->hide();
}
