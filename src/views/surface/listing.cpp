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
    : QScrollArea64{parent}, m_context{ctx} {
    m_surface = rd_surface_create(ctx, RD_RF_DEFAULT);
    m_popup = new SurfacePopup(ctx, this);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setCursor(Qt::ArrowCursor);
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

    connect(this->verticalScrollBar(), &QScrollBar64::sliderMoved, this,
            [&](quint64 value) {
                RDAddressSpace aspace = rd_get_address_space(m_context);
                if(rd_surface_render(m_surface, aspace.start + value))
                    this->viewport()->update();
            });

    connect(this->verticalScrollBar(), &QScrollBar64::actionTriggered, this,
            [&](int action) {
                QScrollBar64* vbar = this->verticalScrollBar();

                switch(action) {
                    case QAbstractSlider::SliderSingleStepAdd: {
                        if(!rd_surface_scroll(m_surface, vbar->singleStep()))
                            return;

                        break;
                    }

                    case QAbstractSlider::SliderSingleStepSub: {
                        if(!rd_surface_scroll(m_surface, -vbar->singleStep()))
                            return;

                        break;
                    }

                    case QAbstractSlider::SliderPageStepAdd: {
                        if(!rd_surface_scroll(m_surface, this->visible_rows()))
                            return;

                        break;
                    }

                    case QAbstractSlider::SliderPageStepSub: {
                        if(!rd_surface_scroll(m_surface, -this->visible_rows()))
                            return;

                        break;
                    }

                    default: break;
                }

                this->viewport()->update();
                this->sync_scrollbars();
            });

    connect(this->verticalScrollBar(), &QScrollBar64::sliderReleased, this,
            [&]() { this->sync_scrollbars(); });

    connect(this->verticalScrollBar(), &QScrollBar64::actionTriggered, this,
            [&](int) { rd_surface_clear_selection(m_surface); });

    this->update_scrollbars();
    this->jump_to_ep();
}

SurfaceListing::~SurfaceListing() {
    rd_surface_destroy(m_surface);
    m_surface = nullptr;
}

bool SurfaceListing::has_selection() const {
    return rd_surface_has_selection(m_surface);
}

RDRenderMode SurfaceListing::get_mode() const {
    return rd_surface_get_mode(m_surface);
}

void SurfaceListing::set_mode(RDRenderMode m) {
    rd_surface_set_mode(m_surface, m);
    this->viewport()->update();
}

bool SurfaceListing::set_position(int row, int col) {
    return rd_surface_set_pos(m_surface, row, col);
}

bool SurfaceListing::select(int row, int col) {
    return rd_surface_select(m_surface, row, col);
}

bool SurfaceListing::select_all() {
    rd_surface_set_pos(m_surface, 0, 0);

    if(!rd_surface_select(m_surface, this->visible_rows(),
                          this->visible_columns()))
        return false;

    this->invalidate();
    return true;
}

bool SurfaceListing::go_back() {
    if(rd_surface_go_back(m_surface)) {
        this->viewport()->update();
        this->sync_scrollbars();
        Q_EMIT history_updated();
        return true;
    }

    return false;
}
bool SurfaceListing::go_forward() {
    if(rd_surface_go_forward(m_surface)) {
        this->viewport()->update();
        this->sync_scrollbars();
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

    this->viewport()->update();
    this->sync_scrollbars();
    Q_EMIT history_updated();
}

void SurfaceListing::jump_to_ep() {
    RDAddress ep;
    if(!rd_get_entry_point(m_context, &ep)) return;
    this->jump_to(ep);
    this->clear_history();
}

bool SurfaceListing::invalidate() {
    if(!rd_surface_repaint(m_surface)) return false;

    this->viewport()->update();
    this->sync_scrollbars();
    return true;
}

void SurfaceListing::mouseDoubleClickEvent(QMouseEvent* e) {
    if(e->button() == Qt::LeftButton) {
        if(!this->follow_under_cursor()) {
            RDSurfacePos p = this->get_surface_coords(e->pos());
            if(rd_surface_select_word(m_surface, p.row, p.col))
                this->invalidate();
        }

        e->accept();
    }
}

void SurfaceListing::mousePressEvent(QMouseEvent* e) {
    if(m_surface) {
        switch(e->button()) {
            case Qt::LeftButton: {
                RDSurfacePos p = this->get_surface_coords(e->pos());
                rd_surface_set_pos(m_surface, p.row, p.col);
                Q_EMIT history_updated();
                this->invalidate();
                break;
            }

            case Qt::BackButton: this->go_back(); break;
            case Qt::ForwardButton: this->go_forward(); break;
            default: return;
        }

        e->accept();
    }
}

void SurfaceListing::mouseMoveEvent(QMouseEvent* e) {
    if(m_surface && (e->buttons() == Qt::LeftButton)) {
        RDSurfacePos p = this->get_surface_coords(e->pos());
        if(rd_surface_select(m_surface, p.row, p.col)) this->invalidate();
        e->accept();
        return;
    }
}

void SurfaceListing::resizeEvent(QResizeEvent* e) {
    e->accept();

    rd_surface_set_min_columns(m_surface, this->visible_columns());

    rd_surface_set_max_rows(m_surface,
                            static_cast<usize>(this->visible_rows()));

    this->invalidate();
    this->update_scrollbars();
}

void SurfaceListing::wheelEvent(QWheelEvent* e) {
    if(!m_surface) return;

    e->accept();
    QPoint delta = e->angleDelta();

    if(delta.y()) {
        rd_surface_clear_selection(m_surface);

        QScrollBar64* vbar = this->verticalScrollBar();
        int const Y_DELTA_SIGN = delta.y() / qAbs(delta.y());

        if(!rd_surface_scroll(m_surface, -Y_DELTA_SIGN * vbar->singleStep()))
            return;

        this->viewport()->update();
        this->sync_scrollbars();
    }
}

void SurfaceListing::keyPressEvent(QKeyEvent* e) {
    if(!utils::handle_key_press(this, e)) {
        auto [row, col] = this->get_position();

        if(e->matches(QKeySequence::MoveToNextLine)) {
            if(row + 1 < this->visible_rows()) {
                this->set_position(row + 1, col);
            }
            else {
                if(!rd_surface_scroll(m_surface, 1)) return;
            }
        }
        else if(e->matches(QKeySequence::MoveToPreviousLine)) {
            if(row - 1 >= 0) {
                this->set_position(row - 1, col);
            }
            else {
                if(!rd_surface_scroll(m_surface, -1)) return;
            }
        }
        else if(e->matches(QKeySequence::MoveToEndOfLine)) {
            rd_surface_set_pos(m_surface, row, this->visible_columns());
        }
        else if(e->matches(QKeySequence::MoveToNextPage)) {
            if(!rd_surface_scroll(m_surface, this->visible_rows())) return;
        }
        else if(e->matches(QKeySequence::MoveToPreviousPage)) {
            if(!rd_surface_scroll(m_surface, -this->visible_rows())) return;
        }
        else if(e->matches(QKeySequence::MoveToStartOfDocument)) {
            rd_surface_set_pos(m_surface, 0, col);
        }
        else if(e->matches(QKeySequence::MoveToEndOfDocument)) {
            rd_surface_set_pos(m_surface, this->visible_rows(), col);
        }
        else if(e->matches(QKeySequence::SelectEndOfLine)) {
            rd_surface_select(m_surface, row, this->visible_columns());
        }
        else if(e->matches(QKeySequence::SelectEndOfDocument)) {
            rd_surface_select(m_surface, this->visible_rows() - row,
                              this->visible_columns());
        }
        else {
            QScrollArea64::keyPressEvent(e);
            return;
        }
    }

    this->invalidate();
}

void SurfaceListing::focusInEvent(QFocusEvent* e) {
    QScrollArea64::focusInEvent(e);

    if(m_surface) {
        rd_surface_set_cursor_visible(m_surface, true);
        this->invalidate();
    }
}

void SurfaceListing::focusOutEvent(QFocusEvent* e) {
    QScrollArea64::focusInEvent(e);

    if(m_surface) {
        rd_surface_set_cursor_visible(m_surface, false);
        this->invalidate();
    }
}

void SurfaceListing::paintEvent(QPaintEvent* e) {
    if(!m_surface) {
        QScrollArea64::paintEvent(e);
        return;
    }

    quint64 start_x = this->horizontalScrollBar()->value();
    QPainter p{this->viewport()};
    surface_renderer::render(&p, m_surface, rd_surface_get_row_count(m_surface),
                             static_cast<usize>(start_x));

    Q_EMIT render_completed();
    statusbar::set_address(this);
}

bool SurfaceListing::event(QEvent* event) {
    if(m_surface && event->type() == QEvent::ToolTip) {
        auto* helpevent = static_cast<QHelpEvent*>(event);
        this->show_popup(helpevent->pos());
        return true;
    }

    return QScrollArea64::event(event);
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
    return qFloor(this->viewport()->height() / surface_renderer::cell_height());
}

RDSurfacePos SurfaceListing::get_surface_coords(QPoint pt) const {
    return surface_renderer::hit_test(pt, this->horizontalScrollBar()->value());
}

RDSurfacePos SurfaceListing::get_position() const {
    return rd_surface_get_pos(m_surface);
}

std::optional<ISurfaceRange> SurfaceListing::get_selected_range() const {
    RDAddress start, end;
    if(!rd_surface_get_selected_range(m_surface, &start, &end))
        return std::nullopt;

    return ISurfaceRange{start, end};
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

std::optional<RDCellData> SurfaceListing::get_cell_data_under_cursor() const {
    RDCellData cd;
    if(rd_surface_get_cell_data_under_cursor(m_surface, &cd)) return cd;
    return std::nullopt;
}

QString SurfaceListing::get_selected_text() const {
    return QString::fromUtf8(rd_surface_get_selected_text(m_surface));
}

bool SurfaceListing::follow_under_cursor() {
    RDAddress address;

    if(rd_surface_get_address_under_cursor(m_surface, &address) &&
       address != this->get_current_address()) {
        this->jump_to(address);
        return true;
    }

    return false;
}

void SurfaceListing::update_scrollbars() {
    RDAddressSpace addrspace = rd_get_address_space(m_context);
    QScrollBar64* vbar = this->verticalScrollBar();
    QScrollBar64* hbar = this->horizontalScrollBar();

    vbar->setRange(0, static_cast<quint64>(addrspace.size));
    vbar->setPageStep(rd_surface_get_byte_span(m_surface));

    bool needs_hscroll =
        rd_surface_get_max_column(m_surface) > this->visible_columns();
    hbar->setMaximum(needs_hscroll ? rd_surface_get_max_column(m_surface) : 0);
}

void SurfaceListing::sync_scrollbars() {
    RDAddressSpace space = rd_get_address_space(m_context);
    RDAddress startaddr;

    if(!rd_surface_get_first_address(m_surface, &startaddr))
        startaddr = space.start;

    QScrollBar64* vbar = this->verticalScrollBar();
    QScrollBar64* hbar = this->horizontalScrollBar();

    QSignalBlocker blk1{this->verticalScrollBar()};
    QSignalBlocker blk2{this->horizontalScrollBar()};

    vbar->setValue(startaddr - space.start);
    vbar->setPageStep(rd_surface_get_byte_span(m_surface));
    m_last_vscroll = vbar->value();

    bool needs_hscroll =
        hbar->value() > 0 ||
        (rd_surface_get_max_column(m_surface) > this->visible_columns());
    hbar->setMaximum(rd_surface_get_max_column(m_surface));
    this->setHorizontalScrollBarVisible(needs_hscroll);
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
