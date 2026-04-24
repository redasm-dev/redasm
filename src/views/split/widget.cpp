#include "widget.h"
#include "support/fontawesome.h"
#include "support/utils.h"
#include "views/split/view.h"
#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>

SplitWidget::SplitWidget(SplitView* view): m_view{view} {
    m_tbactions = new QToolBar();
    m_tbactions->setIconSize({16, 16});
    m_tbactions->setToolButtonStyle(Qt::ToolButtonIconOnly);

    m_widget =
        view->split_delegate()->create_widget(this, view->current_split());
    this->create_default_buttons();

    auto* vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(m_tbactions);

    if(m_widget) vbox->addWidget(m_widget);

    connect(m_view, &SplitView::count_changed, this,
            &SplitWidget::update_close_button);

    m_view->inc_count();
    view->set_selected_widget(this);
}

SplitWidget::~SplitWidget() { m_view->dec_count(); }

void SplitWidget::focusInEvent(QFocusEvent* event) {
    m_view->set_selected_widget(this);
    QWidget::focusInEvent(event);
}

QAction* SplitWidget::action(int idx) const {
    auto actions = m_tbactions->actions();
    return actions.at(idx);
}

QAction* SplitWidget::add_button(const QIcon& icon) {
    return this->add_button(new QAction(icon, QString{}, m_tbactions));
}

QAction* SplitWidget::add_button(QAction* action) {

    if(m_actfirstdefault)
        m_tbactions->insertAction(m_actfirstdefault, action);
    else
        m_tbactions->addAction(action);

    return action;
}

void SplitWidget::create_default_buttons() {
    auto* title = new QLabel();
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    if(m_widget) title->setText(m_widget->windowTitle());

    m_actfirstdefault = m_tbactions->addWidget(title);

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

void SplitWidget::split_horizontally() { this->split(Qt::Horizontal); }
void SplitWidget::split_vertically() { this->split(Qt::Vertical); }

void SplitWidget::open_in_dialog() {
    auto* dlg = new QDialog(m_view);
    dlg->setWindowFlag(Qt::Tool);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    auto* vbox = new QVBoxLayout(dlg);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    vbox->addWidget(new SplitView(m_view->split_delegate()));

    dlg->show();
}

void SplitWidget::close_widget() {
    auto* psplitter = qobject_cast<QSplitter*>(this->parentWidget());

    if(psplitter) {
        QList<int> sizes = psplitter->sizes();
        sizes.removeAt(psplitter->indexOf(this));
        this->deleteLater();
        psplitter->setSizes(sizes);
    }
}

void SplitWidget::update_close_button() {
    m_actclose->setVisible(m_view->count() > 1);
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
        ssplitter->addWidget(new SplitWidget(m_view));

        sizes.append(sizes[index] / 2);
        psplitter->setSizes(sizes);
    }
    else
        psplitter->addWidget(new SplitWidget(m_view));
}
