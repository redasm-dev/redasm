#include "feedbacktoolbutton.h"
#include "support/fontawesome.h"
#include "support/themeprovider.h"
#include <QTimer>

const char* const FEEDBACKTOOL_IN_PROGRESS = "__redasm_feedbacktool__";
constexpr int FEEDBACKTOOL_INTERVAL = 1000;

FeedbackToolButton::FeedbackToolButton(QWidget* parent)
    : QToolButton{parent}, m_feedbackinterval{FEEDBACKTOOL_INTERVAL} {
    m_feedbackicon =
        FA_ICON_COLOR(0xf00c, theme_provider::color(RD_THEME_SUCCESS));

    QObject::connect(this, &QToolButton::clicked, this, [&]() {
        if(!this->property(FEEDBACKTOOL_IN_PROGRESS).isNull()) return;

        Q_EMIT feedback();

        QIcon icon = this->icon();
        this->setProperty(FEEDBACKTOOL_IN_PROGRESS, true);
        this->setIcon(m_feedbackicon);

        QTimer::singleShot(m_feedbackinterval, [&, icon]() {
            this->setIcon(icon);
            this->setProperty(FEEDBACKTOOL_IN_PROGRESS, QVariant{});
        });
    });
}
