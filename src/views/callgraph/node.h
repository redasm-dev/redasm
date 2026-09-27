#pragma once

#include "views/graph/node.h"
#include <redasm/redasm.h>

class CallGraphNode: public GraphViewNode {
    Q_OBJECT

public:
    explicit CallGraphNode(RDContext* ctx, const RDCallGraph* g, RDGraphNode n,
                           QWidget* parent = nullptr);
    [[nodiscard]] QSize size() const override;
    void render(QPainter* p, usize state) override;

private:
    [[nodiscard]] QString get_node_text() const;

protected:
    void mousedoubleclick_event(QMouseEvent* e) override;

Q_SIGNALS:
    void address_requested(RDAddress address);

private:
    RDContext* m_context;
    const RDCallGraph* m_graph;
};
