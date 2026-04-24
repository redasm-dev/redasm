#pragma once

// Widget based on x64dbg's DisassemblerGraphView
//   https://github.com/x64dbg/x64dbg/blob/development/src/gui/Src/Gui/DisassemblerGraphView.h
//   https://github.com/x64dbg/x64dbg/blob/development/src/gui/Src/Gui/DisassemblerGraphView.cpp

#include "views/graph/node.h"
#include <QAbstractScrollArea>
#include <QList>
#include <QVector>
#include <functional>
#include <redasm/redasm.h>
#include <unordered_map>

namespace std {

template<>
struct hash<RDGraphEdge> {
    size_t operator()(const RDGraphEdge& edge) const {
        return edge.src ^ edge.dst;
    }
};

template<>
struct equal_to<RDGraphEdge> {
    bool operator()(const RDGraphEdge& edge1, const RDGraphEdge& edge2) const {
        return (edge1.src == edge2.src) && (edge1.dst == edge2.dst);
    }
};

} // namespace std

class GraphView: public QAbstractScrollArea {
    Q_OBJECT

public:
    explicit GraphView(QWidget* parent = nullptr);
    void set_graph(RDGraph* graph);
    void set_selected_node(const GraphViewNode* item);
    void set_focus_on_selection(bool b);
    void focus_root_block();
    GraphViewNode* selected_item() const;
    RDGraph* graph() const;

public Q_SLOTS:
    void focus_selected_block();
    void update_graph();

protected:
    GraphViewNode* node_from_pos(const QPointF& pt,
                                 QPoint* itempos = nullptr) const;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent*) override;
    void showEvent(QShowEvent* e) override;
    void focus_block(const GraphViewNode* item, bool force = false);
    virtual void selected_item_changed_event();
    virtual GraphViewNode* create_node(RDGraphNode n, const RDGraph* g) = 0;
    virtual void update_edge(const RDGraphEdge&);
    virtual void update_node(GraphViewNode*);
    virtual void compute_layout();
    virtual void begin_compute();
    virtual void end_compute();

private:
    void zoom_out(const QPointF& cursorpos);
    void zoom_in(const QPointF& cursorpos);
    void adjust_size(int vpw, int vph, const QPointF& cursorpos = QPoint(),
                     bool fit = false);
    void precompute_arrow(const RDGraphEdge& e);
    void precompute_line(const RDGraphEdge& e);
    bool update_selected_item(QMouseEvent* e, QPoint* itempos = nullptr);

Q_SIGNALS:
    void selected_item_changed();

protected:
    RDGraph* m_graph{nullptr};
    QHash<RDGraphNode, GraphViewNode*> m_nodes;

private:
    GraphViewNode* m_selecteditem{nullptr};
    std::unordered_map<RDGraphEdge, QVector<QLine>> m_lines;
    std::unordered_map<RDGraphEdge, QPolygon> m_arrows;
    QPoint m_renderoffset, m_scrollbase;
    QSize m_rendersize;
    qreal m_scalefactor{1.0}, m_scalestep{0.1}, m_prevscalefactor{0};
    qreal m_scalemin{0}, m_scalemax{5.0};
    int m_scaledirection{0}, m_scaleboost{1};
    bool m_viewportready{false}, m_scrollmode{true};
    bool m_focusonselection{false};
};
