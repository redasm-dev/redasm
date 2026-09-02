#include "scheduler.h"
#include "statusbar.h"
#include <QTimer>

// static constexpr int BURST_BUDGET_MS = 16; // Fastest
static constexpr int BURST_BUDGET_MS = 8; // Balanced
// static constexpr int BURST_BUDGET_MS = 4;  // Smoothest

static constexpr int NOTIFY_INTERVAL_MS = 100;

Scheduler::Scheduler(RDContext* ctx, QObject* parent)
    : QObject{parent}, m_context{ctx} {
    m_notify_timer.start();
}

void Scheduler::schedule() {
    QTimer::singleShot(0, this, [&]() {
        m_burst_timer.restart();

        while(m_burst_timer.elapsed() < BURST_BUDGET_MS) {
            if(!rd_step(m_context, &m_status)) {
                Q_EMIT yield_requested();
                return;
            }
        }

        bool notify = m_notify_timer.elapsed() >= NOTIFY_INTERVAL_MS;

        if(notify) {
            m_notify_timer.restart();
            Q_EMIT yield_requested();
        }

        if(!m_pause) this->schedule();
    });
}

void Scheduler::toggle_pause() {
    m_pause = !m_pause;

    if(!m_pause) {
        this->schedule();
        statusbar::set_busy_status();
    }
    else
        statusbar::set_pause_status();
}
