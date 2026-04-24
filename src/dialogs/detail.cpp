#include "detail.h"
#include "support/surfacerenderer.h"

DetailDialog::DetailDialog(QWidget* parent): QDialog{parent}, m_ui{this} {
    m_ui.tbdetail->setFont(surface_renderer::get_font());
}

void DetailDialog::set_html(const QString& s) { // NOLINT
    m_ui.tbdetail->setHtml(s);
}
