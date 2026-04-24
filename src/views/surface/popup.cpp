#include "popup.h"
#include "support/surfacerenderer.h"
#include "support/utils.h"
#include <QPainter>
#include <QWheelEvent>

namespace {

constexpr int POPUP_MARGIN = 16;

}

SurfacePopup::SurfacePopup(RDContext* ctx, QWidget* parent): QWidget{parent} {
    this->setFont(surface_renderer::get_font());
    this->setCursor(Qt::ArrowCursor);
    this->setWindowFlags(Qt::Popup);
    this->setMouseTracking(true);
    this->setMinimumHeight(0);
    this->setMinimumWidth(0);

    // this->setAutoFillBackground(true);
    // QPalette p = this->palette();
    // p.setColor(QPalette::Window, themeprovider::color(RD_THEME_BACKGROUND));
    // this->setPalette(p);

    m_surface = rd_surface_create(ctx, RD_RENDERER_POPUP);
}

SurfacePopup::~SurfacePopup() { rd_surface_destroy(m_surface); }

bool SurfacePopup::popup(RDAddress address, const QString& hlword) {
    if(!rd_surface_jump_to(m_surface, address)) return false;

    QPoint pt = QCursor::pos();
    pt.rx() += POPUP_MARGIN;
    pt.ry() += POPUP_MARGIN;

    rd_surface_set_highlight_word(m_surface, qUtf8Printable(hlword));

    this->move(pt);
    this->render();
    this->show();
    return true;
}

void SurfacePopup::more_rows() {
    m_nrows++;
    this->render();
}

void SurfacePopup::less_rows() {
    if(m_nrows == 1) return;

    m_nrows--;
    this->render();
}

void SurfacePopup::render() {
    if(!m_surface) return;

    rd_surface_render(m_surface, m_nrows);

    // get actually renderer rows
    usize len = 0, n = rd_surface_get_row_count(m_surface);

    for(usize i = 0; i < n + 1; i++) {
        RDRowSlice row = rd_surface_get_row(m_surface, i);
        len = qMax(len, row.length);
    }

    this->resize(qFloor(len * surface_renderer::cell_width()),
                 qFloor(n * surface_renderer::cell_height()));

    this->update();
}

void SurfacePopup::mouseMoveEvent(QMouseEvent* event) {
    if(m_lastpos != event->globalPosition()) {
        this->hide();
        event->accept();
    }
    else
        QWidget::mouseMoveEvent(event);
}

void SurfacePopup::wheelEvent(QWheelEvent* event) {
    m_lastpos = event->globalPosition();
    QPoint delta = event->angleDelta();

    if(delta.y() > 0)
        this->less_rows();
    else
        this->more_rows();

    event->accept();
}

void SurfacePopup::paintEvent(QPaintEvent* e) {
    if(!m_surface) {
        QWidget::paintEvent(e);
        return;
    }

    QPainter painter{this};
    surface_renderer::render(&painter, m_surface, 0, m_nrows);
}
