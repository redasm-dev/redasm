#include "surfacetarget.h"
#include "support/utils.h"
#include "views/surface/graph/view.h"
#include "views/surface/listing.h"
#include <QMenu>

namespace surface {

namespace target {

namespace {

enum class View {
    ANY,
    LISTING,
    GRAPH,
};

using Predicate = bool (*)(const Target&);

struct Rule {
    actions::Type type;
    Predicate enabled{nullptr};
    bool targeted{false};
    View view{View::ANY};
    bool shortcut{true};
};

bool always(const Target&) { return true; }

constexpr bool TARGETED = true;
constexpr Rule MENU_SEP = {actions::NONE};

// clang-format off
const Rule RULES[] = { // NOLINT
    {actions::COPY,              [](const Target& t) { return t.has_selection; }},
    {actions::SELECT_ALL,        always},
    MENU_SEP,
    {actions::XREFS_TO,          [](const Target& t) { return t.address && rd_has_refs_to(t.ctx, *t.address); }, TARGETED},
    {actions::RENAME,            [](const Target& t) { return t.address.has_value(); }, TARGETED},
    MENU_SEP,
    {actions::OP_AS_ADDRESS,     [](const Target& t) { return t.has_operand() && t.cell->operand.value.kind == RD_OP_IMM && rd_is_address(t.ctx, t.cell->operand.value.imm); }},
    {actions::OP_AS_IMMEDIATE,   [](const Target& t) { return t.has_operand() && t.cell->operand.value.kind == RD_OP_ADDR; }},
    {actions::COMMENT,           [](const Target& t) { return t.has_flags; }, TARGETED},
    {actions::DO_UNDEFINE,       [](const Target& t) { return t.range || (t.has_flags && t.has_value() && !t.is_unknown()); }, TARGETED},
    {actions::DO_CODE,           [](const Target& t) { return t.is_executable() && t.has_flags && t.has_value() && !t.is_code(); }, TARGETED},
    {actions::DO_DATA,           [](const Target& t) { return t.has_flags && t.has_value() && !t.is_data(); }, TARGETED},
    {actions::DO_BRANCH,         [](const Target& t) { return t.has_flags && t.has_value() && !t.is_code(); }, TARGETED},
    MENU_SEP,
    {actions::SWITCH_TO_GRAPH,   always, false, View::LISTING},
    {actions::SWITCH_TO_LISTING, always, false, View::GRAPH},
    {actions::SWITCH_TO_HEX,     [](const Target& t) { return t.address.has_value(); }, TARGETED},
    MENU_SEP,
    {actions::PATCH_INSTRUCTION, [](const Target& t) { return t.is_executable() && t.is_code(); }, TARGETED},
    {actions::XREFS_EDIT,        [](const Target& t) { return t.has_flags && !t.is_unknown(); }, TARGETED},
    {actions::CREATE_FUNCTION,   [](const Target& t) { return t.is_executable() && t.has_flags && !t.is_data() && !t.is_func_start() && !t.in_func; }, TARGETED},
    MENU_SEP,
    {actions::GOTO,              always, false, View::ANY, false},
    {actions::OPEN_CALLGRAPH,    [](const Target& t) { return t.in_func; }, TARGETED},
    {actions::OPEN_DETAILS,      [](const Target& t) { return t.address.has_value(); }, TARGETED},
};
// clang-format on

} // namespace

Target resolve(ISurface* surface) {
    Target t;
    t.ctx = surface->context();
    t.row_address = surface->get_current_address();
    t.range = surface->get_selected_range();
    t.cell = surface->get_cell_data_under_cursor();
    t.has_selection = surface->has_selection();

    // Clicked word has priority over the row.
    if(auto word = surface->get_address_under_cursor(); word) {
        t.address = word;
        t.from_cursor = true;
    }
    else {
        t.address = t.row_address;
    }

    if(t.address) {
        t.segment = rd_segment_find(t.ctx, *t.address);
        t.has_flags = t.segment && rd_get_flags(t.ctx, *t.address, &t.flags);
        t.in_func = rd_function_find(t.ctx, *t.address, nullptr);
    }

    return t;
}

bool is_enabled(actions::Type type, const Target& t) {
    for(const Rule& r : RULES) {
        if(r.type == type) return r.enabled && r.enabled(t);
    }

    return true;
}

} // namespace target

QMenu* create_menu(ISurface* surface) {
    QWidget* w = surface->to_widget();
    const bool IS_LISTING = qobject_cast<SurfaceListing*>(w) != nullptr;
    const bool IS_GRAPH = qobject_cast<SurfaceGraphView*>(w) != nullptr;

    auto* menu = new QMenu(w);
    menu->setSeparatorsCollapsible(true); // hidden actions leave no gaps

    struct Bound {
        QAction* action;
        const target::Rule* rule;
        QString text;
    };

    std::vector<Bound> bound;

    for(const target::Rule& r : target::RULES) {
        if(r.type == actions::NONE) {
            menu->addSeparator();
            continue;
        }

        if(r.view == target::View::LISTING && !IS_LISTING) continue;
        if(r.view == target::View::GRAPH && !IS_GRAPH) continue;

        QAction* a = actions::create(r.type, w, r.shortcut);
        menu->addAction(a);
        bound.push_back({a, &r, a->text()});
    }

    QObject::connect(menu, &QMenu::aboutToShow, w, [=]() {
        const target::Target TGT = target::resolve(surface);
        QString name;

        if(TGT.from_cursor && TGT.address) {
            const char* n = rd_get_name(TGT.ctx, *TGT.address);
            name = n ? QString::fromUtf8(n) : utils::to_hex(*TGT.address);
        }

        for(const Bound& b : bound) {
            b.action->setVisible(b.rule->enabled(TGT));

            if(b.rule->targeted && !name.isEmpty())
                b.action->setText(QString{"%1 @ %2"}.arg(b.text, name));
        }
    });

    QObject::connect(menu, &QMenu::aboutToHide, w, [=]() {
        for(const Bound& b : bound) {
            b.action->setVisible(true);
            b.action->setText(b.text);
        }
    });

    return menu;
}

} // namespace surface
