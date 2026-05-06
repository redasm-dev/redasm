#include "context.h"
#include "statusbar.h"

static constexpr int THROTTLE_INTERVAL_MS = 100;

ContextView::ContextView(RDContext* ctx, QWidget* parent)
    : QWidget{parent}, m_ui{ctx, this}, m_context{ctx} {

    m_functionsmodel = new FunctionsModel(ctx, this);
    m_ui.tvfunctions->setModel(m_functionsmodel);
    m_ui.tvfunctions->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    m_throttle_timer.start();

    connect(m_ui.tvfunctions, &QTreeView::doubleClicked, this,
            [&](const QModelIndex& index) {
                RDAddress address = m_functionsmodel->address(index);
                m_ui.splitview->surface()->jump_to(address);
            });
}

ContextView::~ContextView() { rd_destroy(m_context); }

void ContextView::toggle_active() {
    if(m_busy) m_active = !m_active;
}

void ContextView::report_status() {
    if(m_status->is_busy && m_throttle_timer.elapsed() < THROTTLE_INTERVAL_MS)
        return;

    m_throttle_timer.restart();

    if(m_status->segment && m_status->address.has_value) {
        statusbar::set_status_text(
            QString{"Step: %1  Calls: %2  Jumps: %3  Address: %4"}
                .arg(m_status->step)
                .arg(m_status->pending_calls)
                .arg(m_status->pending_jumps)
                .arg(m_status->address.value, m_status->segment->unit * 2, 16,
                     QLatin1Char('0')));
    }
    else {
        statusbar::set_status_text(QString{"Step: %1  Calls: %2  Jumps: %3"}
                                       .arg(m_status->step)
                                       .arg(m_status->pending_calls)
                                       .arg(m_status->pending_jumps));
    }
}

bool ContextView::loop() {
    if(m_busy) {
        if(m_active) {
            m_busy = rd_step(m_context, &m_status);

            if(m_status->is_listing_changed) {
                m_functionsmodel->resync();
                m_ui.splitview->surface()->invalidate();
            }

            if(!m_busy) {
                statusbar::set_ready_status();
                statusbar::check_problems(m_context);
                m_ui.splitview->surface()->jump_to_ep();
            }
            else {
                statusbar::set_busy_status();
                this->report_status();
            }
        }
        else
            statusbar::set_pause_status();
    }

    return m_busy;
}
