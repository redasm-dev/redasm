#include "feedbackpushbutton.h"
#include "support/fontawesome.h"
#include "support/themeprovider.h"
#include <QTimer>

const char* const FEEDBACKPUSH_IN_PROGRESS = "__redasm_feedbackpush__";
constexpr int FEEDBACKPUSH_INTERVAL = 1000;

FeedbackPushButton::FeedbackPushButton(QWidget* parent)
    : QPushButton{parent}, m_feedbackinterval{FEEDBACKPUSH_INTERVAL} {
    m_feedbackicon =
        FA_ICON_COLOR(0xf00c, theme_provider::color(RD_THEME_SUCCESS));

    m_feedbacktext = "Done";

    QObject::connect(this, &QPushButton::clicked, this, [&]() {
        if(!this->property(FEEDBACKPUSH_IN_PROGRESS).isNull()) return;

        Q_EMIT feedback();

        QIcon icon = this->icon();
        QString text = this->text();
        this->setProperty(FEEDBACKPUSH_IN_PROGRESS, true);
        this->setIcon(m_feedbackicon);
        this->setText(m_feedbacktext);

        QTimer::singleShot(m_feedbackinterval, [&, icon, text]() {
            this->setIcon(icon);
            this->setText(text);
            this->setProperty(FEEDBACKPUSH_IN_PROGRESS, QVariant{});
        });
    });
}
