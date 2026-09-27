#pragma once

#include "views/graph/view.h"
#include <redasm/redasm.h>

class CallGraph: public GraphView {
    Q_OBJECT

public:
    explicit CallGraph(RDContext* ctx, RDAddress address,
                       QWidget* parent = nullptr);
    ~CallGraph() override;
    RDGraph* graph() const override;

protected:
    GraphViewNode* create_node(RDGraphNode n, const RDGraph*) override;
    void compute_layout() override;

Q_SIGNALS:
    void address_requested(RDAddress address);

private:
    RDContext* m_context;
    RDCallGraph* m_graph;
};
