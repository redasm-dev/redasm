#pragma once

#include "views/graph/view.h"
#include "views/surface/graph/node.h"
#include "views/surface/isurface.h"
#include "views/surface/popup.h"
#include <QMenu>
#include <optional>

class SurfaceGraph: public GraphView, public ISurface {
    Q_OBJECT
    Q_INTERFACES(ISurface)

public:
    // clang-format off
    explicit SurfaceGraph(RDContext* ctx, QWidget* parent = nullptr);
    ~SurfaceGraph() override;
    [[nodiscard]] QWidget* to_widget() override { return this; }
    [[nodiscard]] RDContext* context() override { return m_context; }
    [[nodiscard]] RDSurfacePos get_position() const override;
    [[nodiscard]] QString get_selected_text() const override;
    [[nodiscard]] std::optional<RDAddress> get_current_address() const override;
    [[nodiscard]] std::optional<RDAddress> get_address_under_cursor() const override;
    [[nodiscard]] bool can_go_back() const override;
    [[nodiscard]] bool can_go_forward() const override;
    [[nodiscard]] bool has_selection() const override;
    [[nodiscard]] bool has_rdil() const override;
    void jump_to_ep() override;
    void jump_to(RDAddress address) override;
    void invalidate() override;
    void set_rdil(bool b) override;
    void set_position(int row, int col) override;
    void select(int row, int col) override;
    // clang-format on

public Q_SLOTS:
    bool go_back() override;
    bool go_forward() override;
    void clear_history() override;

protected:
    GraphViewNode* create_node(RDGraphNode n, const RDGraph* g) override;
    void compute_layout() override;
    void end_compute() override;
    void update_node(GraphViewNode*) override;
    void keyPressEvent(QKeyEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
    bool event(QEvent* event) override;

private:
    SurfaceGraphNode* find_node(RDAddress address);
    SurfaceGraphNode* find_node_at_cursor();
    void show_popup(const QPoint& pt);

Q_SIGNALS:
    void history_updated();
    void switch_view();

private:
    RDContext* m_context;
    SurfacePopup* m_popup;
    QMenu* m_menu;
    RDSurfaceGraph* m_surface{nullptr};
    bool m_functionchanged{false};
};
