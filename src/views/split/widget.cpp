#include "widget.h"
#include "support/fontawesome.h"
#include "support/utils.h"
#include "views/split/view.h"
#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>

namespace {

void _splitwidget_collapse_empty_splitters(QSplitter* splitter) {
    while(splitter && splitter->count() == 1) {
        auto* pparent = qobject_cast<QSplitter*>(splitter->parentWidget());

        // top level SplitView itself, nothing to collapse into
        if(!pparent) break;

        int idx = pparent->indexOf(splitter);
        QWidget* remaining = splitter->widget(0);

        pparent->replaceWidget(idx, remaining); // keeps pparent's sizes intact
        splitter->deleteLater();

        // keep collapsing upward if this made pparent single-child too
        splitter = pparent;
    }
}

} // namespace

SplitWidget::SplitWidget(SplitView* view, SplitWidget* splitfrom)
    : m_view{view} {
    m_tbactions = new QToolBar();
    m_tbactions->setIconSize({16, 16});
    m_tbactions->setToolButtonStyle(Qt::ToolButtonIconOnly);

    m_widget = view->split_delegate()->create_widget(this, splitfrom);

    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(m_tbactions);

    if(m_widget) vbox->addWidget(m_widget);

    connect(m_view, &SplitView::count_changed, this,
            &SplitWidget::update_close_button);

    m_view->inc_count();
    view->set_selected_widget(this);

    connect(qApp, &QApplication::focusChanged, this,
            [this](QWidget*, QWidget* now) {
                if(now && this->isAncestorOf(now))
                    m_view->set_selected_widget(this);
            });
}

SplitWidget::~SplitWidget() { m_view->dec_count(); }

QAction* SplitWidget::action(int idx) const {
    auto actions = m_tbactions->actions();
    return actions.at(idx);
}

QAction* SplitWidget::add_button(const QIcon& icon) {
    return this->add_button(new QAction(icon, QString{}, m_tbactions));
}

QAction* SplitWidget::add_button(QAction* action) {
    m_tbactions->addAction(action);
    return action;
}

QAction* SplitWidget::add_widget(QWidget* w) {
    return m_tbactions->addWidget(w);
}

void SplitWidget::add_spacer() {
    auto* spacer = new QWidget(m_tbactions);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tbactions->addWidget(spacer);
}

void SplitWidget::create_default_buttons() {
    QAction* acthsplit = m_tbactions->addAction(FA_ICON(0xf105), {});
    QAction* actvsplit = m_tbactions->addAction(FA_ICON(0xf107), {});
    QAction* actdlgopen = m_tbactions->addAction(FA_ICON(0xf2d2), {});
    m_actclose = m_tbactions->addAction(FA_ICON(0xf00d), {});

    // clang-format off
    connect(acthsplit, &QAction::triggered, this, &SplitWidget::split_horizontally);
    connect(actvsplit, &QAction::triggered, this, &SplitWidget::split_vertically);
    connect(actdlgopen, &QAction::triggered, this, &SplitWidget::open_in_dialog);
    connect(m_actclose, &QAction::triggered, this, &SplitWidget::close_widget);
    // clang-format on
}

void SplitWidget::create_title() {
    auto* title = new QLabel();
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    if(m_widget) title->setText(m_widget->windowTitle());
    m_tbactions->addWidget(title);
}

void SplitWidget::split_horizontally() { this->split(Qt::Horizontal); }
void SplitWidget::split_vertically() { this->split(Qt::Vertical); }

void SplitWidget::open_in_dialog() {
    auto* dlg = new QDialog(m_view);
    dlg->setWindowFlag(Qt::Tool);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->resize(1000, 600);

    auto* vbox = new QVBoxLayout(dlg);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(new SplitView(m_view->split_delegate()));

    dlg->show();
}

void SplitWidget::close_widget() {
    auto* psplitter = qobject_cast<QSplitter*>(this->parentWidget());
    if(!psplitter) return;

    QList<int> sizes = psplitter->sizes();
    sizes.removeAt(psplitter->indexOf(this));

    if(m_view->current_split() == this) {
        if(auto* sibling = this->find_sibling_to_focus()) {
            if(sibling->m_widget)
                sibling->m_widget->setFocus();
            else
                m_view->set_selected_widget(sibling);
        }
        else
            m_view->set_selected_widget(nullptr);
    }

    connect(this, &QObject::destroyed, psplitter, [psplitter]() {
        _splitwidget_collapse_empty_splitters(psplitter);
    });

    this->deleteLater();
    psplitter->setSizes(sizes);
}

void SplitWidget::update_close_button() {
    if(m_actclose) m_actclose->setVisible(m_view->count() > 1);
}

SplitWidget* SplitWidget::find_sibling_to_focus() const {
    if(auto* psplitter = qobject_cast<QSplitter*>(this->parentWidget())) {
        for(int i = 0; i < psplitter->count(); i++) {
            QWidget* w = psplitter->widget(i);
            if(w == this) continue;
            if(auto* sw = qobject_cast<SplitWidget*>(w)) return sw;
            if(auto* sw = w->findChild<SplitWidget*>()) return sw;
        }
    }

    // fallback: any other split still alive in the view
    for(auto* sw : m_view->findChildren<SplitWidget*>()) {
        if(sw != this) return sw;
    }

    return nullptr;
}

void SplitWidget::split(Qt::Orientation orientation) {
    auto* psplitter = qobject_cast<QSplitter*>(this->parentWidget());
    if(!psplitter) return;

    QList<int> sizes = psplitter->sizes();

    if(psplitter->orientation() != orientation) {
        auto* ssplitter = new QSplitter(orientation);
        int index = psplitter->indexOf(this);
        psplitter->insertWidget(index, ssplitter);
        ssplitter->addWidget(this);
        ssplitter->addWidget(new SplitWidget(m_view, this));

        int half = sizes[index] / 2;
        ssplitter->setSizes({half, half});
        psplitter->setSizes(sizes);
    }
    else
        psplitter->addWidget(new SplitWidget(m_view, this));
}
