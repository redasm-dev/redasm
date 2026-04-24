#pragma once

#include "views/surface/isurface.h"
#include "views/surface/popup.h"
#include <QAbstractScrollArea>
#include <QMenu>
#include <optional>
#include <redasm/redasm.h>

class SurfaceListing: public QAbstractScrollArea, public ISurface {
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
    [[nodiscard]] std::optional<RDAddress> get_current_address() const override;
    [[nodiscard]] std::optional<RDAddress> get_address_under_cursor() const override;
    [[nodiscard]] QString get_selected_text() const override;
    [[nodiscard]] bool can_go_back() const override;
    [[nodiscard]] bool can_go_forward() const override;
    [[nodiscard]] bool has_selection() const override;
    [[nodiscard]] bool has_rdil() const override;
    [[nodiscard]] int visible_columns() const;
    [[nodiscard]] int visible_rows() const;
    void set_rdil(bool v) override;
    void set_position(int row, int col) override;
    void select(int row, int col) override;
    // clang-format on

public Q_SLOTS:
    bool go_back() override;
    bool go_forward() override;
    void jump_to(RDAddress address) override;
    void jump_to_ep() override;
    void clear_history() override;
    void invalidate() override;

protected:
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    bool event(QEvent* event) override;

private:
    [[nodiscard]] RDSurfacePos get_surface_coords(QPoint pt) const;
    [[nodiscard]] usize get_length() const;
    bool follow_under_cursor();
    void update_scrollbars();
    void sync_location();
    void show_popup(const QPoint& pt);

Q_SIGNALS:
    void history_updated();
    void render_completed();
    void switch_view();

private:
    SurfacePopup* m_popup;
    RDContext* m_context{nullptr};
    RDSurface* m_surface{nullptr};
    QMenu* m_menu;
};
