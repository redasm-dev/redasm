#include "context.h"
#include "statusbar.h"
#include <QTimer>

static constexpr int THROTTLE_INTERVAL_MS = 100;

ContextView::ContextView(RDContext* ctx, QWidget* parent)
    : QWidget{parent}, m_ui{ctx, this}, m_context{ctx} {

    m_functionsmodel = new FunctionsModel(ctx, this);
    m_ui.tvfunctions->setModel(m_functionsmodel);
    m_ui.tvfunctions->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    m_analysis_timer.setInterval(0);
    m_throttle_timer.start();

    statusbar::set_busy_status();

    connect(&m_analysis_timer, &QTimer::timeout, this,
            &ContextView::analyze_step);

    connect(m_ui.tvfunctions, &QTreeView::doubleClicked, this,
            [&](const QModelIndex& index) {
                RDAddress address = m_functionsmodel->address(index);
                m_ui.splitview->surface()->jump_to(address);
            });
}

ContextView::~ContextView() { rd_destroy(m_context); }

void ContextView::toggle_pause() {
    m_pause = !m_pause;

    if(!m_pause) {
        this->schedule_step();
        statusbar::set_busy_status();
    }
    else {
        m_analysis_timer.stop();
        statusbar::set_pause_status();
    }
}

void ContextView::analyze_step() {
    if(rd_step(m_context, &m_status)) {
        bool notify = m_status.is_listing_changed ||
                      m_throttle_timer.elapsed() >= THROTTLE_INTERVAL_MS;
        if(notify) {
            m_throttle_timer.restart();
            this->check_status();
        }
    }
    else {
        m_analysis_timer.stop();
        this->check_status();
    }
}

void ContextView::check_status() {
    if(m_status.is_listing_changed) {
        m_functionsmodel->resync();
        m_ui.splitview->surface()->invalidate();
    }

    if(!m_status.is_busy) {
        statusbar::set_ready_status();
        statusbar::check_problems(m_context);
        m_ui.splitview->surface()->jump_to_ep();
    }

    if(m_status.segment && m_status.address.has_value) {
        statusbar::set_status_text(
            QString{"Step: %1  Calls: %2  Jumps: %3  Address: %4"}
                .arg(m_status.step)
                .arg(m_status.pending_calls)
                .arg(m_status.pending_jumps)
                .arg(m_status.address.value, m_status.segment->unit * 2, 16,
                     QLatin1Char('0')));
    }
    else {
        statusbar::set_status_text(QString{"Step: %1  Calls: %2  Jumps: %3"}
                                       .arg(m_status.step)
                                       .arg(m_status.pending_calls)
                                       .arg(m_status.pending_jumps));
    }
}

void ContextView::schedule_step() {
    if(!m_analysis_timer.isActive()) m_analysis_timer.start();
}
