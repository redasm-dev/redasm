#pragma once

#include "views/surface/isurface.h"
#include "views/surface/popup.h"
#include "widgets/scroll/qscrollarea64.h"
#include <QMenu>
#include <optional>
#include <redasm/redasm.h>

class SurfaceListing: public QScrollArea64, public ISurface {
    Q_OBJECT
    Q_INTERFACES(ISurface)

public:
    // clang-format off
    explicit SurfaceListing(RDContext* ctx, QWidget* parent = nullptr);
    ~SurfaceListing() override;
    [[nodiscard]] RDSurface* handle() const { return m_surface; }
    [[nodiscard]] QWidget* to_widget() override { return this; }
    [[nodiscard]] RDContext* context() override { return m_context; }
    [[nodiscard]] RDSurfacePos get_position() const override;
    [[nodiscard]] std::optional<ISurfaceRange> get_selected_range() const override;
    [[nodiscard]] std::optional<RDAddress> get_current_address() const override;
    [[nodiscard]] std::optional<RDAddress> get_address_under_cursor() const override;
    [[nodiscard]] std::optional<RDCellData> get_cell_data_under_cursor() const override;
    [[nodiscard]] QString get_selected_text() const override;
    [[nodiscard]] bool can_go_back() const override;
    [[nodiscard]] bool can_go_forward() const override;
    [[nodiscard]] bool has_selection() const override;
    [[nodiscard]] RDRenderMode get_mode() const override;
    [[nodiscard]] int visible_columns() const;
    [[nodiscard]] int visible_rows() const;
    void set_mode(RDRenderMode m) override;
    bool set_position(int row, int col) override;
    bool select(int row, int col) override;
    bool select_all() override;
    // clang-format on

public Q_SLOTS:
    bool go_back() override;
    bool go_forward() override;
    void jump_to(RDAddress address) override;
    void jump_to_ep() override;
    void clear_history() override;
    bool invalidate() override;

protected:
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    bool event(QEvent* event) override;

private:
    [[nodiscard]] RDSurfacePos get_surface_coords(QPoint pt) const;
    bool follow_under_cursor();
    void update_scrollbars();
    void sync_scrollbars();
    void show_popup(const QPoint& pt);

Q_SIGNALS:
    void view_requested(ISurface::ViewRequest req,
                        std::optional<RDAddress> address = std::nullopt);
    void history_updated();
    void render_completed();

private:
    SurfacePopup* m_popup;
    RDContext* m_context{nullptr};
    RDSurface* m_surface{nullptr};
    QMenu* m_menu;
    quint64 m_last_vscroll{0};
};
