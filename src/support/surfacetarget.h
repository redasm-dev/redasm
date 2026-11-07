#pragma once

// POLICY
// ------
// Every address-based action acts on ONE target, resolved at the moment it is
// shown or triggered:
//
//   addr = address of the word under the cursor, if any
//          (get_address_under_cursor)
//        else address of the row at the caret
//          (get_current_address)
//
// A clicked word always wins over the row: when it does, targeted actions are
// labelled "@ name" in the menu.
//
// Exceptions:
//   - operand actions act on the operand under the cursor of the ROW
//     instruction, so they use `row`, never `addr`.
//   - undefine prefers the selected range when there is one.
//   - view switches keep the row.
//
// A rule's predicate decides both menu visibility and whether the handler
// runs.

#include "support/actions.h"
#include "views/surface/isurface.h"
#include <optional>
#include <redasm/redasm.h>

namespace surface {

namespace target {

struct Target {
    RDContext* context{};

    std::optional<RDAddress> address;
    std::optional<RDAddress> row_address;
    bool from_cursor{false};

    std::optional<ISurfaceRange> range;
    std::optional<RDCellData> cell;
    bool has_selection{false};

    const RDSegment* segment{};
    RDFlags flags{};
    bool has_flags{false};
    bool in_func{false};

    // clang-format off
    [[nodiscard]] bool is_executable() const { return this->segment && rd_segment_has_perm(this->segment, RD_SP_X); }
    [[nodiscard]] bool has_operand() const   { return this->row_address && this->cell && this->cell->operand.index != -1; }
    [[nodiscard]] bool has_value() const     { return this->has_flags && rd_flags_get_value(this->flags, nullptr); }
    [[nodiscard]] bool is_unknown() const    { return this->has_flags && rd_flags_has_unknown(this->flags); }
    [[nodiscard]] bool is_code() const       { return this->has_flags && rd_flags_has_code(this->flags); }
    [[nodiscard]] bool is_data() const       { return this->has_flags && rd_flags_has_data(this->flags); }
    [[nodiscard]] bool is_func_start() const { return this->has_flags && rd_flags_has_func(this->flags); }
    // clang-format on
};

[[nodiscard]] Target resolve(ISurface* surface);
[[nodiscard]] bool is_enabled(actions::Type type, const Target& t);

} // namespace target

QMenu* create_menu(ISurface* surface);

} // namespace surface
