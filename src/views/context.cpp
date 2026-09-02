#include "context.h"
#include "statusbar.h"
#include "views/surface/graph/graph.h"
#include "views/surface/hex.h"
#include "views/surface/listing.h"

ContextView::ContextView(RDContext* ctx, QWidget* parent)
    : QWidget{parent}, m_scheduler{new Scheduler{ctx, this}},
      m_ui{ctx, m_scheduler, this}, m_context{ctx} {

    connect(m_scheduler, &Scheduler::yield_requested, this,
            &ContextView::check_status);

    m_functionsmodel = new FunctionsModel(ctx, this);
    m_ui.tvfunctions->setModel(m_functionsmodel);
    m_ui.tvfunctions->header()->moveSection(0, 1);
    m_ui.tvfunctions->header()->setSectionResizeMode(
        0, QHeaderView::ResizeToContents);
    m_ui.tvfunctions->header()->setSectionResizeMode(1, QHeaderView::Stretch);

    statusbar::set_busy_status();

    connect(m_ui.tvfunctions, &QTreeView::doubleClicked, this,
            [&](const QModelIndex& index) {
                RDAddress address = m_functionsmodel->address(index);
                if(m_ui.splitview->surface())
                    m_ui.splitview->surface()->jump_to(address);
            });
}

ContextView::~ContextView() { rd_destroy(m_context); }

void ContextView::show_segment_regs() {
    if(m_dlg_sregs) {
        m_dlg_sregs->raise();
        m_dlg_sregs->activateWindow();
        return;
    }

    m_dlg_sregs = new SegmentRegsDialog(m_context, this);

    connect(m_dlg_sregs, &SegmentRegsDialog::destroyed, this,
            [&]() { m_dlg_sregs = nullptr; });

    m_dlg_sregs->show();
}

void ContextView::toggle_pause() { m_scheduler->toggle_pause(); }
void ContextView::schedule_step() { m_scheduler->schedule(); }

void ContextView::check_status() {
    m_ui.splitview->surface()->invalidate();

    const RDWorkerStatus* status = m_scheduler->status();

    if(!status->is_busy) {
        statusbar::set_ready_status();
        statusbar::check_problems(m_context);
        if(!status->reconcile) m_ui.splitview->surface()->jump_to_ep();
    }

    if(status->segment && status->address.has_value) {
        statusbar::set_status_text(
            QString{"Step: %1  Calls: %2  Jumps: %3  Address: %4"}
                .arg(status->step)
                .arg(status->pending_calls)
                .arg(status->pending_jumps)
                .arg(status->address.value,
                     static_cast<qsizetype>(rd_get_ptr_size(m_context)) * 2, 16,
                     QLatin1Char('0')));
    }
    else {
        statusbar::set_status_text(QString{"Step: %1  Calls: %2  Jumps: %3"}
                                       .arg(status->step)
                                       .arg(status->pending_calls)
                                       .arg(status->pending_jumps));
    }
}

void ContextView::invalidate() { // NOLINT
    this->surface()->invalidate();
}

void ContextView::handle_view_requested(ISurface::ViewRequest req, // NOLINT
                                        std::optional<RDAddress> address) {
    QWidget* w = this->surface()->to_widget();
    if(!w) return;

    if(auto* l = qobject_cast<SurfaceListing*>(w); l)
        Q_EMIT l->view_requested(req, address);
    else if(auto* g = qobject_cast<SurfaceGraph*>(w); g)
        Q_EMIT g->view_requested(req, address);
    else if(auto* h = qobject_cast<HexView*>(w); h)
        Q_EMIT h->view_requested(req, address);
}
