#pragma once

#include "views/callgraph/graph.h"
#include <QDialog>
#include <QVBoxLayout>
#include <redasm/redasm.h>

namespace ui {

struct CallGraphDialog {
    CallGraph* callgraph;

    explicit CallGraphDialog(RDContext* ctx, RDAddress address, QDialog* self) {
        self->resize(1000, 600);
        self->setAttribute(Qt::WA_DeleteOnClose);

        this->callgraph = new CallGraph(ctx, address);

        auto* vbox = new QVBoxLayout(self);
        vbox->setContentsMargins(0, 0, 0, 0);
        vbox->addWidget(this->callgraph);
    }
};

} // namespace ui
