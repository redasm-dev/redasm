#include "welcome.h"
#include "delegates/recentfiles.h"
#include "models/recentfiles.h"
#include "support/actions.h"
#include <redasm/version.h>

namespace {

void bind_button_to_action(QPushButton* button, actions::Type t) {
    QAction* act = actions::get(t);

    button->setFlat(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setIconSize({28, 28});
    button->setIcon(act->icon());
    button->setText(act->text());

    QObject::connect(button, &QPushButton::clicked, act, &QAction::trigger);
}

void style_social_button(QPushButton* button, actions::Type t) {
    const QString SOCIAL_STYLESHEET = QString(R"(
        QPushButton:hover { border: 1px solid; }
        QPushButton { text-align: left; }
    )");

    button->setStyleSheet(SOCIAL_STYLESHEET);
    bind_button_to_action(button, t);
}

} // namespace

WelcomeView::WelcomeView(QWidget* parent): DashboardView{parent}, m_ui{this} {
    this->apply_logo(m_ui.lblbrand);

    m_ui.lblversion->setText(QString{"<b>Version:</b> %1"}.arg(RD_VERSION_STR));

    m_ui.lvrecents->viewport()->setAttribute(Qt::WA_Hover);
    m_ui.lvrecents->viewport()->setBackgroundRole(QPalette::Window);

    m_ui.pbopen->setCursor(Qt::PointingHandCursor);
    m_ui.pbsettings->setCursor(Qt::PointingHandCursor);
    m_ui.pbabout->setCursor(Qt::PointingHandCursor);

    this->make_bordered(m_ui.pbopen);
    this->make_bordered(m_ui.pbsettings);
    this->make_bordered(m_ui.pbabout);

    style_social_button(m_ui.pbopenhome, actions::OPEN_HOME);
    style_social_button(m_ui.pbopenx, actions::OPEN_X);
    style_social_button(m_ui.pbopentelegram, actions::OPEN_TELEGRAM);
    style_social_button(m_ui.pbopenreddit, actions::OPEN_REDDIT);
    style_social_button(m_ui.pbopengithub, actions::OPEN_GITHUB);
    bind_button_to_action(m_ui.pbsettings, actions::OPEN_SETTINGS);
    bind_button_to_action(m_ui.pbabout, actions::OPEN_ABOUT);

    auto* recentfilesmodel = new RecentFilesModel(m_ui.lvrecents);
    recentfilesmodel->update();

    m_ui.lvrecents->setItemDelegate(new RecentFilesDelegate(m_ui.lvrecents));
    m_ui.lvrecents->setModel(recentfilesmodel);

    connect(m_ui.pbopen, &QPushButton::clicked, this,
            &WelcomeView::open_requested);
    connect(m_ui.lvrecents, &QListView::clicked, this,
            &WelcomeView::on_file_selected);
}

void WelcomeView::on_file_selected(const QModelIndex& index) {
    const auto* recentfilesmodel =
        static_cast<const RecentFilesModel*>(index.model());

    Q_EMIT file_selected(recentfilesmodel->file_path(index));
}
