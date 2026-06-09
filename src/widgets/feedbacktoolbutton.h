#pragma once

#include <QToolButton>

class FeedbackToolButton: public QToolButton {
    Q_OBJECT

public:
    explicit FeedbackToolButton(QWidget* parent = nullptr);
    [[nodiscard]] int feedback_interval() const { return m_feedbackinterval; }
    void set_feedback_interval(int v) { m_feedbackinterval = v; }
    [[nodiscard]] const QIcon& feedback_icon() const { return m_feedbackicon; }
    void set_feedback_icon(const QIcon& v) { m_feedbackicon = v; }

Q_SIGNALS:
    void feedback();

private:
    int m_feedbackinterval;
    QIcon m_feedbackicon;
};
