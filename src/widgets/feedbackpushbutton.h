#pragma once

#include <QPushButton>

class FeedbackPushButton: public QPushButton {
    Q_OBJECT

public:
    explicit FeedbackPushButton(QWidget* parent = nullptr);

    // clang-format off
    [[nodiscard]] int feedback_interval() const { return m_feedbackinterval; }
    void set_feedback_interval(int v) { m_feedbackinterval = v; }
    [[nodiscard]] const QIcon& feedback_icon() const { return m_feedbackicon; }
    void set_feedback_icon(const QIcon& v) { m_feedbackicon = v; }
    [[nodiscard]] const QString& feedback_text() const { return m_feedbacktext; }
    void set_feedback_text(const QString& v) { m_feedbacktext = v; }
    // clang-format on

Q_SIGNALS:
    void feedback();

private:
    int m_feedbackinterval;
    QIcon m_feedbackicon;
    QString m_feedbacktext;
};
