#pragma once

#include "views/surface/split.h"
#include <QHeaderView>
#include <QSplitter>
#include <QStackedWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <redasm/redasm.h>

namespace ui {

struct ContextView {
    QSplitter* splitter;
    SurfaceSplitView* splitview;
    QTreeView* tvfunctions;

    explicit ContextView(RDContext* ctx, QWidget* self) {
        this->splitview = new SurfaceSplitView(ctx); // FIXME: ctx leaking in ui

        this->tvfunctions = new QTreeView();
        this->tvfunctions->header()->setStretchLastSection(false);
        this->tvfunctions->setUniformRowHeights(true);
        this->tvfunctions->setRootIsDecorated(false);
        this->tvfunctions->setFrameShape(QFrame::NoFrame);

        QPalette p = self->palette();
        QColor bg = p.color(QPalette::Window);
        QColor fg = p.color(QPalette::WindowText);

        const QString STYLE = QString{"QHeaderView::section {"
                                      "    background-color: %1;"
                                      "    color: %2;"
                                      "    padding: 4px;"
                                      "    border: 1px solid %1;"
                                      "    font-weight: bold;"
                                      "}"}
                                  .arg(bg.name())
                                  .arg(fg.name());

        this->tvfunctions->setStyleSheet(STYLE);

        this->splitter = new QSplitter();
        this->splitter->addWidget(this->tvfunctions);
        this->splitter->addWidget(this->splitview);
        this->splitter->setStretchFactor(0, 20);
        this->splitter->setStretchFactor(1, 80);

        auto* vbox = new QVBoxLayout(self);
        vbox->setContentsMargins(0, 0, 0, 0);
        vbox->addWidget(this->splitter);
    }
};

} // namespace ui
