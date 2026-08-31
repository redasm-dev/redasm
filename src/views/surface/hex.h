#pragma once

#include "views/surface/isurface.h"
#include <QHexView/qhexview.h>
#include <QWidget>
#include <redasm/redasm.h>

class HexView: public QWidget, public ISurface {
    Q_OBJECT
    Q_INTERFACES(ISurface)

public:
    explicit HexView(RDContext* ctx, QWidget* parent = nullptr);
    [[nodiscard]] QHexView* viewport() { return m_hexview; }

    // clang-format off
public: // ISurface implementation
    void clear_history() override;
    void jump_to_ep() override;
    void jump_to(RDAddress address) override;
    void set_mode(RDRenderMode m) override;
    bool invalidate() override;
    bool go_back() override;
    bool go_forward() override;
    bool set_position(int row, int col) override;
    bool select(int row, int col) override;
    bool select_all() override;
    [[nodiscard]] RDRenderMode get_mode() const override;
    [[nodiscard]] bool has_selection() const override;
    [[nodiscard]] bool can_go_back() const override;
    [[nodiscard]] bool can_go_forward() const override;
    [[nodiscard]] std::optional<ISurfaceRange> get_selected_range() const override;
    [[nodiscard]] std::optional<RDAddress> get_current_address() const override;
    [[nodiscard]] std::optional<RDAddress> get_address_under_cursor() const override;
    [[nodiscard]] std::optional<RDCellData> get_cell_data_under_cursor() const override;
    [[nodiscard]] QString get_selected_text() const override;
    [[nodiscard]] RDSurfacePos get_position() const override;
    [[nodiscard]] RDContext* context() override { return m_context; }
    [[nodiscard]] QWidget* to_widget() override { return this; }
    // clang-format on

private:
    void create_popup_menu();

Q_SIGNALS:
    void view_requested(ISurface::ViewRequest req,
                        std::optional<RDAddress> address = std::nullopt);

private:
    const RDSegment* m_segment{nullptr};
    RDContext* m_context;
    QHexView* m_hexview;
    QMenu* m_popupmenu{nullptr};
};
