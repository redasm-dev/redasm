#include "view.h"
#include "views/split/widget.h"

SplitView::SplitView(SplitDelegate* delegate, QWidget* parent)
    : QSplitter{Qt::Horizontal, parent}, m_delegate{delegate} {
    if(!m_delegate) qFatal("Invalid SplitView delegate");

    if(!m_delegate->parent()) // Take ownership if doesn't have parents
        m_delegate->setParent(this);

    this->addWidget(new SplitWidget(this));
}

void SplitView::set_selected_widget(SplitWidget* sw) {
    if(m_currsplit == sw) return;

    m_currsplit = sw;
    Q_EMIT selected_changed();
}

void SplitView::inc_count() {
    m_count++;
    Q_EMIT count_changed();
}

void SplitView::dec_count() {
    m_count--;
    Q_EMIT count_changed();
}
