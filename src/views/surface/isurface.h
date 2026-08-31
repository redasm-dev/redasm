#pragma once

#include <QtPlugin>
#include <optional>
#include <redasm/redasm.h>

using ISurfaceRange = std::pair<RDAddress, RDAddress>;

class ISurface {
public:
    enum class ViewRequest {
        LISTING,
        GRAPH,
        HEX,
    };

public:
    // clang-format off
    virtual ~ISurface() = default;
    virtual void clear_history() = 0;
    virtual void jump_to_ep() = 0;
    virtual void jump_to(RDAddress address) = 0;
    virtual void set_mode(RDRenderMode m) = 0;
    virtual bool invalidate() = 0;
    virtual bool go_back() = 0;
    virtual bool go_forward() = 0;
    virtual bool set_position(int row, int col) = 0;
    virtual bool select(int row, int col) = 0;
    virtual bool select_all() = 0;
    [[nodiscard]] virtual RDRenderMode get_mode() const = 0;
    [[nodiscard]] virtual bool has_selection() const = 0;
    [[nodiscard]] virtual bool can_go_back() const = 0;
    [[nodiscard]] virtual bool can_go_forward() const = 0;
    [[nodiscard]] virtual std::optional<ISurfaceRange> get_selected_range() const = 0;
    [[nodiscard]] virtual std::optional<RDAddress> get_current_address() const = 0;
    [[nodiscard]] virtual std::optional<RDAddress> get_address_under_cursor() const = 0;
    [[nodiscard]] virtual std::optional<RDCellData> get_cell_data_under_cursor() const = 0;
    [[nodiscard]] virtual QString get_selected_text() const = 0;
    [[nodiscard]] virtual RDSurfacePos get_position() const = 0;
    [[nodiscard]] virtual RDContext* context() = 0;
    [[nodiscard]] virtual QWidget* to_widget() = 0;
    // clang-format on
};

Q_DECLARE_INTERFACE(ISurface, "com.redasm.ISurface")
