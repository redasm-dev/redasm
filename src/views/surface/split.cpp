#include "split.h"
#include "support/actions.h"
#include "support/fontawesome.h"
#include "support/themeprovider.h"
#include "support/utils.h"
#include "views/split/widget.h"
#include "views/surface/graph/graph.h"
#include "views/surface/view.h"
#include <QComboBox>
#include <QToolButton>

namespace {

ISurface* _splitwidget_getcurrentsurface(SplitWidget* split) {
    if(!split) return nullptr;

    auto* stackw = qobject_cast<QStackedWidget*>(split->widget());

    if(stackw) {
        if(auto* w = qobject_cast<SurfaceView*>(stackw->currentWidget()); w)
            return w->listing();

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

SurfaceSplitDelegate::SurfaceSplitDelegate(RDContext* ctx, QObject* parent)
    : SplitDelegate{parent}, m_context{ctx} {}

QWidget* SurfaceSplitDelegate::create_widget(SplitWidget* current,
                                             SplitWidget* split) {
    QAction* actback = current->add_button(
        FA_ICON_COLOR(0xf053, theme_provider::color(RD_THEME_SUCCESS)));
    QAction* actforward = current->add_button(
        FA_ICON_COLOR(0xf054, theme_provider::color(RD_THEME_SUCCESS)));
    current->add_button(actions::get(actions::GOTO));

    auto* stack = new QStackedWidget();
    auto* surfaceview = new SurfaceView(m_context);
    auto* surfacegraph = new SurfaceGraph(m_context);
    stack->addWidget(surfaceview);
    stack->addWidget(surfacegraph);

    FeedbackToolButton* tbscreenshot = utils::create_screenshot_button(stack);
    current->add_widget(tbscreenshot);

    auto* cbrendermode =
        static_cast<QComboBox*>(current->add_widget(new QComboBox()));

    cbrendermode->setFrame(false);
    cbrendermode->addItems(QStringList{"NORMAL", "RDIL", "FLAGS"});

    QObject::connect(cbrendermode, &QComboBox::currentIndexChanged, this,
                     [current](int idx) {
                         ISurface* s = _splitwidget_getcurrentsurface(current);
                         if(s) {
                             s->set_mode(static_cast<RDRenderMode>(idx));
                             s->to_widget()->setFocus();
                         }
                     });

    // Initialize Actions
    actback->setEnabled(surfaceview->listing()->can_go_back());
    actforward->setEnabled(surfaceview->listing()->can_go_forward());

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
            [surfaceview, actback, actforward]() {
                if(surfaceview->isVisible()) { // Ignore spurious signals
                    actback->setEnabled(surfaceview->listing()->can_go_back());
                    actforward->setEnabled(
                        surfaceview->listing()->can_go_forward());
                }
            });

    connect(surfacegraph, &SurfaceGraph::history_updated,
            [surfacegraph, actback, actforward]() {
                if(surfacegraph->isVisible()) { // Ignore spurious signals
                    actback->setEnabled(surfacegraph->can_go_back());
                    actforward->setEnabled(surfacegraph->can_go_forward());
                }
            });

    connect(surfaceview, &SurfaceView::switch_view, this,
            [stack, surfaceview, surfacegraph, cbrendermode]() {
                auto address = surfaceview->listing()->get_current_address();
                if(!address) return;

                stack->setCurrentWidget(surfacegraph);
                surfacegraph->set_mode(
                    static_cast<RDRenderMode>(cbrendermode->currentIndex()));
                surfacegraph->jump_to(*address);
            });

    connect(surfacegraph, &SurfaceGraph::switch_view, this,
            [stack, surfaceview, surfacegraph, cbrendermode]() {
                auto address = surfacegraph->get_current_address();
                if(!address) return;

                stack->setCurrentWidget(surfaceview);
                surfaceview->listing()->set_mode(
                    static_cast<RDRenderMode>(cbrendermode->currentIndex()));
                surfaceview->listing()->jump_to(*address);
            });

    connect(actback, &QAction::triggered, this, [current]() {
        if(ISurface* s = _splitwidget_getcurrentsurface(current)) s->go_back();
    });

    connect(actforward, &QAction::triggered, this, [current]() {
        if(ISurface* s = _splitwidget_getcurrentsurface(current))
            s->go_forward();
    });

    return stack;
}

SurfaceSplitView::SurfaceSplitView(RDContext* ctx, QWidget* parent)
    : SplitView{new SurfaceSplitDelegate(ctx), parent} {}

ISurface* SurfaceSplitView::surface() const {
    return _splitwidget_getcurrentsurface(this->current_split());
}
