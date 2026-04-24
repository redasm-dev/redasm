#include "context.h"
#include "statusbar.h"

ContextView::ContextView(RDContext* ctx, QWidget* parent)
    : QWidget{parent}, m_ui{ctx, this}, m_context{ctx} {

    m_functionsmodel = new FunctionsModel(ctx, this);
    m_ui.tvfunctions->setModel(m_functionsmodel);
    m_ui.tvfunctions->header()->setSectionResizeMode(0, QHeaderView::Stretch);

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
    QString s;

    s += QString::fromWCharArray(L"<b>Step: </b>%1\u00A0\u00A0")
             .arg(m_status->step);

    s += QString::fromWCharArray(L"<b>Pending Calls: </b>%1\u00A0\u00A0")
             .arg(m_status->pending_calls);

    s += QString::fromWCharArray(L"<b>Pending Jumps: </b>%1\u00A0\u00A0")
             .arg(m_status->pending_jumps);

    if(m_status->segment && m_status->address.has_value) {
        s += QString::fromWCharArray(L"<b>Address: </b>%1\u00A0\u00A0")
                 .arg(m_status->address.value, m_status->segment->unit * 2, 16,
                      QLatin1Char('0'));
    }

    statusbar::set_status_text(s);
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
