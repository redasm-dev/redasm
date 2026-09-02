#include "split.h"
#include "support/fontawesome.h"
#include "support/themeprovider.h"
#include "support/utils.h"
#include "views/split/widget.h"
#include "views/surface/graph/graph.h"
#include "views/surface/hex.h"
#include "views/surface/view.h"
#include <QComboBox>
#include <QToolButton>

namespace {

// default is NORMAL, which is 1
constexpr std::array<RDRenderMode, 3> RENDER_MODES = {
    RD_RM_FLAGS,
    RD_RM_NORMAL,
    RD_RM_RDIL,
};

ISurface* _splitwidget_getcurrentsurface(SplitWidget* split) {
    if(!split) return nullptr;

    auto* stackw = qobject_cast<QStackedWidget*>(split->widget());

    if(stackw) {
        if(auto* w = qobject_cast<SurfaceView*>(stackw->currentWidget()); w)
            return w->listing();
        if(auto* w = qobject_cast<HexView*>(stackw->currentWidget()); w)
            return w;

        return qobject_cast<ISurface*>(stackw->currentWidget());
    }

    return nullptr;
}

ISurface* _splitwidget_findsurfacelisting(SplitWidget* split) {
    if(!split) return nullptr;

    auto* stackw = qobject_cast<QStackedWidget*>(split->widget());

    if(stackw) {
        for(int i = 0; i < stackw->count(); i++) {
            if(auto* w = qobject_cast<SurfaceView*>(stackw->widget(i)); w)
                return w->listing();
        }
    }

    return nullptr;
}

SurfaceGraph* _splitwidget_findsurfacegraph(SplitWidget* split) {
    if(!split) return nullptr;

    auto* stackw = qobject_cast<QStackedWidget*>(split->widget());

    if(stackw) {
        for(int i = 0; i < stackw->count(); i++) {
            if(auto* w = qobject_cast<SurfaceGraph*>(stackw->widget(i)); w)
                return w;
        }
    }

    return nullptr;
}

} // namespace

SurfaceSplitDelegate::SurfaceSplitDelegate(RDContext* ctx, Scheduler* scheduler,
                                           QObject* parent)
    : SplitDelegate{parent}, m_context{ctx}, m_scheduler{scheduler} {}

QWidget* SurfaceSplitDelegate::create_widget(SplitWidget* current,
                                             SplitWidget* split) {
    QAction* act_back = current->add_button(
        FA_ICON_COLOR(0xf053, theme_provider::color(RD_THEME_SUCCESS)));
    QAction* act_forward = current->add_button(
        FA_ICON_COLOR(0xf054, theme_provider::color(RD_THEME_SUCCESS)));

    auto* stack = new QStackedWidget();
    auto* surfaceview = new SurfaceView(m_context);
    auto* surfacegraph = new SurfaceGraph(m_context);
    auto* hexview = new HexView(m_context);

    stack->addWidget(surfaceview);
    stack->addWidget(surfacegraph);
    stack->addWidget(hexview);

    // subscribe all views to scheduler
    connect(m_scheduler, &Scheduler::yield_requested, surfaceview->listing(),
            &SurfaceListing::invalidate);
    connect(m_scheduler, &Scheduler::yield_requested, surfacegraph,
            &SurfaceGraph::invalidate);
    connect(m_scheduler, &Scheduler::yield_requested, hexview,
            &HexView::invalidate);

    FeedbackToolButton* tbscreenshot = utils::create_screenshot_button(stack);
    current->add_widget(tbscreenshot);

    current->add_spacer();

    auto* cbrendermode = new QComboBox();
    QAction* act_cbrendermode = current->add_widget(cbrendermode);

    cbrendermode->addItem("FLAGS", RD_RM_FLAGS);
    cbrendermode->addItem("NORMAL", RD_RM_NORMAL);
    cbrendermode->addItem("RDIL", RD_RM_RDIL);

    cbrendermode->setFrame(false);
    cbrendermode->setCurrentIndex(1); // NORMAL by default

    auto do_switch_view = [=](ISurface::ViewRequest req,
                              std::optional<RDAddress> address) {
        ISurface* from = _splitwidget_getcurrentsurface(current);
        if(!from) return;

        // discard any edits if coming from HexView
        if(auto* h = qobject_cast<HexView*>(from->to_widget()); h) {
            h->clear_changes();
            m_scheduler->schedule();
        }

        switch(req) {
            case ISurface::ViewRequest::LISTING: {
                stack->setCurrentWidget(surfaceview);
                act_back->setVisible(true);
                act_forward->setVisible(true);
                act_cbrendermode->setVisible(true);

                surfaceview->listing()->set_mode(
                    RENDER_MODES[cbrendermode->currentIndex()]);

                if(address) surfaceview->listing()->jump_to(*address);
                break;
            }

            case ISurface::ViewRequest::GRAPH: {
                stack->setCurrentWidget(surfacegraph);
                act_back->setVisible(true);
                act_forward->setVisible(true);
                act_cbrendermode->setVisible(true);

                surfacegraph->set_mode(
                    RENDER_MODES[cbrendermode->currentIndex()]);

                if(address) surfacegraph->jump_to(*address);
                break;
            }

            case ISurface::ViewRequest::HEX: {
                stack->setCurrentWidget(hexview);
                act_back->setVisible(false);
                act_forward->setVisible(false);
                act_cbrendermode->setVisible(false);

                if(address) hexview->jump_to(*address);
                break;
            }

            default: break;
        }
    };

    QObject::connect(cbrendermode, &QComboBox::currentIndexChanged, this,
                     [current](int idx) {
                         ISurface* s = _splitwidget_getcurrentsurface(current);
                         if(s) {
                             s->set_mode(RENDER_MODES[idx]);
                             s->to_widget()->setFocus();
                         }
                     });

    // Initialize Actions
    act_back->setEnabled(surfaceview->listing()->can_go_back());
    act_forward->setEnabled(surfaceview->listing()->can_go_forward());

    // Sync view location
    if(ISurface* v = _splitwidget_findsurfacelisting(split); v) {
        auto address = v->get_current_address();
        if(address) surfaceview->listing()->jump_to(*address);
    }

    // Sync graph location
    if(ISurface* g = _splitwidget_findsurfacegraph(split); g) {
        auto address = g->get_current_address();
        if(address) surfacegraph->jump_to(*address);
    }

    connect(surfaceview, &SurfaceView::history_updated,
            [surfaceview, act_back, act_forward]() {
                if(surfaceview->isVisible()) { // Ignore spurious signals
                    act_back->setEnabled(surfaceview->listing()->can_go_back());
                    act_forward->setEnabled(
                        surfaceview->listing()->can_go_forward());
                }
            });

    connect(surfacegraph, &SurfaceGraph::history_updated,
            [surfacegraph, act_back, act_forward]() {
                if(surfacegraph->isVisible()) { // Ignore spurious signals
                    act_back->setEnabled(surfacegraph->can_go_back());
                    act_forward->setEnabled(surfacegraph->can_go_forward());
                }
            });

    connect(surfaceview->listing(), &SurfaceListing::view_requested, this,
            [=](ISurface::ViewRequest req, std::optional<RDAddress> address) {
                do_switch_view(req, address);
            });

    connect(surfacegraph, &SurfaceGraph::view_requested, this,
            [=](ISurface::ViewRequest req, std::optional<RDAddress> address) {
                do_switch_view(req, address);
            });

    connect(hexview, &HexView::view_requested, this,
            [=](ISurface::ViewRequest req, std::optional<RDAddress> address) {
                do_switch_view(req, address);
            });

    connect(act_back, &QAction::triggered, this, [current]() {
        if(ISurface* s = _splitwidget_getcurrentsurface(current)) s->go_back();
    });

    connect(act_forward, &QAction::triggered, this, [current]() {
        if(ISurface* s = _splitwidget_getcurrentsurface(current))
            s->go_forward();
    });

    current->create_default_buttons();
    return stack;
}

SurfaceSplitView::SurfaceSplitView(RDContext* ctx, Scheduler* scheduler,
                                   QWidget* parent)
    : SplitView{new SurfaceSplitDelegate(ctx, scheduler), parent} {}

ISurface* SurfaceSplitView::surface() const {
    return _splitwidget_getcurrentsurface(this->current_split());
}
