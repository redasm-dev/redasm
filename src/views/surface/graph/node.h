#pragma once

#include "views/graph/node.h"
#include <redasm/redasm.h>

class SurfaceGraphNode: public GraphViewNode {
    Q_OBJECT

public:
    explicit SurfaceGraphNode(RDSurfaceGraph* surface, const RDFunctionChunk& b,
                              RDGraphNode n, QWidget* parent = nullptr);
    [[nodiscard]] bool contains_address(RDAddress address) const;
    [[nodiscard]] int current_row() const override;
    [[nodiscard]] QSize size() const override;
    void get_surface_pos(const QPointF& pt, RDSurfacePos* pos) const;
    void render(QPainter* painter, usize state) override;
    void update_metrics();

protected:
    void mousedoubleclick_event(QMouseEvent*) override;
    void mousepress_event(QMouseEvent* e) override;
    void mousemove_event(QMouseEvent* e) override;

private:
    [[nodiscard]] int start_row() const;
    [[nodiscard]] int end_row() const;

Q_SIGNALS:
    void follow_requested();

private:
    RDFunctionChunk m_block;
    RDSurfaceGraph* m_surface;
    int m_maxwidth{};
};
