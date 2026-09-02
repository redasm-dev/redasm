#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <redasm/redasm.h>

class Scheduler: public QObject {
    Q_OBJECT

public:
    explicit Scheduler(RDContext* ctx, QObject* parent = nullptr);
    [[nodiscard]] const RDWorkerStatus* status() const { return &m_status; }

public Q_SLOTS:
    void schedule();
    void toggle_pause();

Q_SIGNALS:
    void yield_requested();

private:
    RDContext* m_context;
    RDWorkerStatus m_status{};
    QElapsedTimer m_burst_timer;
    QElapsedTimer m_notify_timer;
    bool m_pause{false};
};
