#include "callgraph.h"
#include "support/utils.h"

CallGraphDialog::CallGraphDialog(RDContext* ctx, RDAddress address,
                                 QWidget* parent)
    : QDialog{parent}, m_ui{ctx, address, this} {
    this->setWindowTitle(
        QString{"CallGraph @ %1"}.arg(utils::to_hex(address, ctx)));

    connect(m_ui.callgraph, &CallGraph::address_requested, this,
            &CallGraphDialog::address_requested);
}
