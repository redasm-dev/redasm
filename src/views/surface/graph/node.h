#pragma once

#include "views/graph/node.h"
#include <redasm/redasm.h>

class SurfaceGraphNode: public GraphViewNode {
    Q_OBJECT

public:
    explicit SurfaceGraphNode(RDSurfaceGraph* surface,
                              const RDFunctionChunk* chunk, RDGraphNode n,
                              QWidget* parent = nullptr);
    [[nodiscard]] bool contains_address(RDAddress address) const;
    [[nodiscard]] int current_row() const override;
    [[nodiscard]] QSize size() const override;
    [[nodiscard]] int start_row() const;
    [[nodiscard]] int end_row() const;
    void get_surface_pos(const QPointF& pt, RDSurfacePos* pos) const;
    void render(QPainter* p, usize state) override;
    void update_metrics();

protected:
    void mousedoubleclick_event(QMouseEvent*) override;
    void mousepress_event(QMouseEvent* e) override;
    void mousemove_event(QMouseEvent* e) override;

Q_SIGNALS:
    void follow_requested();

private:
    const RDFunctionChunk* m_chunk;
    RDSurfaceGraph* m_surface;
    int m_maxwidth{};
};
