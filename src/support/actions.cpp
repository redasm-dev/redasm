#include "actions.h"
#include "dialogs/about.h"
#include "dialogs/branch.h"
#include "dialogs/callgraph.h"
#include "dialogs/detail.h"
#include "dialogs/goto.h"
#include "dialogs/input.h"
#include "dialogs/patch.h"
#include "dialogs/rebase.h"
#include "dialogs/referencesfrom.h"
#include "dialogs/settings.h"
#include "dialogs/table.h"
#include "dialogs/types.h"
#include "mainwindow.h"
#include "models/referencesto.h"
#include "support/fontawesome.h"
#include "support/surfacetarget.h"
#include "support/utils.h"
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QHash>
#include <QInputDialog>
#include <QMessageBox>
#include <cinttypes>
#include <optional>

namespace actions {

namespace {

enum class IconKind { NONE, FA, FAB };

struct ActionDesc {
    IconKind icon_kind;
    unsigned int icon_id;
    QString text;
    QKeyCombination key;
    void (*handler)();
    QString url;
};

MainWindow* g_mainwindow;
QHash<Type, ActionDesc> g_actiontable;

struct SurfaceCall {
    ContextView* context_view;
    surface::target::Target target;
};

// Resolves the active surface's target and applies the SAME rule the context
// menu uses, so a menu click and a keyboard shortcut always agree.
// nullopt -> the action doesn't apply here.
std::optional<SurfaceCall> resolve(Type type) {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return std::nullopt;

    surface::target::Target t = surface::target::resolve(cv->surface());
    if(!surface::target::is_enabled(type, t)) return std::nullopt;
    return SurfaceCall{cv, std::move(t)};
}

const char* const DETAIL_TEMPLATE = R"(
    <b>Address:</b> %1<br>
    <b>Offset:</b> %2<br>
    <br>
    )";

const char* const INSTR_TEMPLATE = R"(
        <b>==== INSTRUCTION ====</b><br>
        <b>Address:</b> %1<br>
        <b>Id:</b> %2<br>
        <b>Flow:</b> %3<br>
        <b>Length:</b> %4<br>
        <b>Delay slots:</b> %5<br>
        <br>
        )";

const char* const OP_TEMPLATE = R"(
        <b>==== OPERAND %1 ====</b><br>
        <b>Type:</b> %2<br>
        <b>Userdata1:</b> %3<br>
        <b>Userdata2:</b> %4<br>
        )";

QString optype_tostring(const RDOperand* op) {
    switch(op->kind) {
        case RD_OP_STUB: return "OP_STUB";
        case RD_OP_CNST: return "OP_CNST";
        case RD_OP_REG: return "OP_REG";
        case RD_OP_IMM: return "OP_IMM";
        case RD_OP_ADDR: return "OP_ADDR";
        case RD_OP_MEM: return "OP_MEM";
        case RD_OP_DISPL: return "OP_DISPL";
        default: break;
    }

    return QString{"#%1"}.arg(op->kind);
}

QString instrflow_tostring(const RDInstruction* instr) {
    QString f;

    switch(instr->flow) {
        case RD_IF_JUMP: f = "IF_JUMP"; break;
        case RD_IF_JUMP_COND: f = "IF_JUMP_COND"; break;
        case RD_IF_CALL: f = "IF_CALL"; break;
        case RD_IF_CALL_COND: f = "IF_CALL_COND"; break;
        case RD_IF_STOP: f = "IF_STOP"; break;
        case RD_IF_NOP: f = "IF_NOP"; break;
        default: f = "IF_NONE"; break;
    }

    if(rd_instr_is_delay_slot(instr)) f.append(" | IS_DSLOT");
    return f;
}

QString xreftype_tostring(const RDXRef& r) {
    switch(r.type) {
        case RD_DR_READ: return "DR_READ";
        case RD_DR_WRITE: return "DR_WRITE";
        case RD_DR_ADDRESS: return "DR_ADDRESS";
        case RD_CR_JUMP: return "CR_JUMP";
        case RD_CR_CALL: return "CR_CALL";
        default: break;
    }

    return {};
}

void show_goto() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;

    auto* dlggoto = new GotoDialog(cv->context(), cv);

    QObject::connect(
        dlggoto, &GotoDialog::accepted, g_mainwindow,
        [cv, dlggoto]() { cv->surface()->jump_to(dlggoto->address); });

    dlggoto->show();
}

void copy() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;
    qApp->clipboard()->setText(cv->surface()->get_selected_text());
}

void select_all() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;
    cv->surface()->select_all();
}

void reanalyze() {
    ContextView* cv = g_mainwindow->context_view();

    if(cv && rd_reanalyze(cv->context())) {
        cv->schedule_step();
        cv->invalidate();
    }
}

void rebase() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;

    auto* dlgrebase = new RebaseDialog(cv->context(), cv);

    QObject::connect(dlgrebase, &RebaseDialog::accepted, cv, [cv, dlgrebase]() {
        if(rd_set_base_address(cv->context(), dlgrebase->address())) {
            cv->invalidate();
            cv->schedule_step();
        }
    });

    dlgrebase->show();
}

void show_details() {
    auto sc = actions::resolve(Type::OPEN_DETAILS);
    if(!sc || !sc->target.address.has_value()) return;

    ContextView* cv = sc->context_view;
    RDAddress address = *sc->target.address;

    RDOffset offset;
    bool has_offset = rd_to_offset(cv->context(), address, &offset);

    QString s = QString{DETAIL_TEMPLATE}
                    .arg(rd_to_hexaddr(cv->context(), address))
                    .arg(has_offset ? utils::to_hex(offset) : "N/A");

    RDInstruction instr;

    if(rd_decode(cv->context(), address, &instr)) {
        QString strinstr = QString{INSTR_TEMPLATE}
                               .arg(rd_to_hexaddr(cv->context(), instr.address))
                               .arg(rd_to_hexaddr(cv->context(), instr.id))
                               .arg(instrflow_tostring(&instr))
                               .arg(rd_to_hexaddr(cv->context(), instr.length))
                               .arg(QString::number(instr.delay_slots));

        rd_foreach_operand(i, op, &instr) {
            QString strop =
                QString{OP_TEMPLATE}
                    .arg(i)
                    .arg(optype_tostring(op))
                    .arg(rd_to_hexaddr(cv->context(), op->userdata1))
                    .arg(rd_to_hexaddr(cv->context(), op->userdata2));

            switch(op->kind) {
                case RD_OP_STUB: break;

                case RD_OP_CNST: {
                    strop.append(
                        QString("<b>cnst:</b> %1<br>")
                            .arg(rd_to_hexaddr(cv->context(), op->cnst)));
                    break;
                }

                case RD_OP_REG: {
                    strop.append(QString("<b>reg:</b> %1<br>").arg(op->reg));
                    break;
                }

                case RD_OP_IMM: {
                    strop.append(
                        QString("<b>imm:</b> %1<br>")
                            .arg(rd_to_hexaddr(cv->context(), op->imm)));
                    break;
                }

                case RD_OP_ADDR: {
                    strop.append(
                        QString("<b>addr:</b> %1<br>")
                            .arg(rd_to_hexaddr(cv->context(), op->addr)));
                    break;
                }

                case RD_OP_MEM: {
                    strop.append(
                        QString("<b>mem:</b> %1<br>")
                            .arg(rd_to_hexaddr(cv->context(), op->mem)));
                    break;
                }

                case RD_OP_DISPL: {
                    strop.append(
                        QString("<b>base:</b> %1<br>")
                            .arg(rd_to_hexaddr(cv->context(), op->displ.base)));
                    strop.append(QString("<b>index:</b> %1<br>")
                                     .arg(rd_to_hexaddr(cv->context(),
                                                        op->displ.index)));
                    strop.append(
                        QString("<b>displ:</b> %1<br>")
                            .arg(rd_to_hexaddr(
                                cv->context(),
                                static_cast<usize>(op->displ.offset))));
                    break;
                }

                default: {
                    strop.append(
                        QString("<b>reg1:</b> %1<br>")
                            .arg(rd_to_hexaddr(cv->context(), op->user.reg1)));
                    strop.append(
                        QString("<b>reg2:</b> %1<br>")
                            .arg(rd_to_hexaddr(cv->context(), op->user.reg2)));
                    strop.append(
                        QString("<b>reg3:</b> %1<br>")
                            .arg(rd_to_hexaddr(cv->context(), op->user.reg3)));
                    strop.append(
                        QString("<b>val1:</b> %1<br>")
                            .arg(rd_to_hexaddr(cv->context(), op->user.val1)));
                    strop.append(
                        QString("<b>val2:</b> %1<br>")
                            .arg(rd_to_hexaddr(cv->context(), op->user.val2)));
                    strop.append(
                        QString("<b>val3:</b> %1<br>")
                            .arg(rd_to_hexaddr(cv->context(), op->user.val3)));
                    break;
                }
            }

            strinstr.append(strop);
        }

        s.append(strinstr);
    }

    RDXRefSlice xrefs = rd_get_xrefs_from(cv->context(), address, RD_XR_NONE);

    if(!rd_slice_is_empty(xrefs)) {
        s.append(QString{"<br><b>==== XREFS FROM %1 ====</b><br>"}.arg(
            rd_to_hexaddr(cv->context(), address)));

        for(usize i = 0; i < rd_slice_length(xrefs); i++) {
            RDXRef r = rd_slice_at(xrefs, i);

            s.append(QString{"[%1]: %2 (%3)<br>"}
                         .arg(i)
                         .arg(rd_to_hexaddr(cv->context(), r.address))
                         .arg(xreftype_tostring(r)));
        }
    }

    xrefs = rd_get_xrefs_to(cv->context(), address, RD_XR_NONE);

    if(!rd_slice_is_empty(xrefs)) {
        s.append(QString{"<br><b>==== XREFS TO %1 ====</b><br>"}.arg(
            rd_to_hexaddr(cv->context(), address)));

        for(usize i = 0; i < rd_slice_length(xrefs); i++) {
            RDXRef r = rd_slice_at(xrefs, i);

            s.append(QString{"[%1]: %2 (%3)<br>"}
                         .arg(i)
                         .arg(rd_to_hexaddr(cv->context(), r.address))
                         .arg(xreftype_tostring(r)));
        }
    }

    auto* dlg = new DetailDialog(cv);
    auto title =
        QString("Details @ %1").arg(rd_to_hexaddr(cv->context(), address));
    dlg->setWindowTitle(title);
    dlg->set_html(s);
    dlg->show();
}

void show_callgraph() {
    auto sc = actions::resolve(Type::OPEN_CALLGRAPH);
    if(!sc || !sc->target.address.has_value()) return;

    ContextView* cv = sc->context_view;
    auto* dlg = new CallGraphDialog(cv->context(), *sc->target.address, cv);

    QObject::connect(dlg, &CallGraphDialog::address_requested, g_mainwindow,
                     [](RDAddress address) {
                         ContextView* curr_cv = g_mainwindow->context_view();
                         if(curr_cv) curr_cv->surface()->jump_to(address);
                     });

    dlg->show();
}

void switch_listing() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;

    cv->handle_view_requested(ISurface::ViewRequest::LISTING,
                              cv->surface()->get_current_address());
}

void switch_graph() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;

    cv->handle_view_requested(ISurface::ViewRequest::GRAPH,
                              cv->surface()->get_current_address());
}

void switch_hex() {
    auto sc = actions::resolve(Type::SWITCH_TO_HEX);
    if(!sc || !sc->target.address.has_value()) return;

    sc->context_view->handle_view_requested(ISurface::ViewRequest::HEX,
                                            sc->target.address);
}

void comment() {
    auto sc = actions::resolve(Type::COMMENT);
    if(!sc || !sc->target.address.has_value()) return;

    ContextView* cv = sc->context_view;
    auto address = sc->target.address;

    bool ok = false;
    const char* cmt = rd_get_comment_inline(cv->context(), *address);

    QString s = input_dialog::get_multi_line_text(
        g_mainwindow, QString{"Comment @ %1"}.arg(utils::to_hex(*address)),
        "Insert comment (or leave empty), CTRL+ENTER to accept:",
        cmt ? cmt : QString{}, &ok);

    if(ok && rd_set_comment_inline(cv->context(), *address, qUtf8Printable(s)))
        cv->invalidate();
}

void xrefs_to() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;

    surface::target::Target t = surface::target::resolve(cv->surface());
    auto address = t.address;
    if(!address) return;

    // Not a silent no-op: tell the user why the shortcut did nothing.
    if(!surface::target::is_enabled(Type::XREFS_TO, t)) {
        RD_LOG_INFO("no xrefs to %" PRIX64, *address);
        return;
    }

    auto* dlg = new TableDialog(
        QString("References to %1").arg(rd_to_hexaddr(cv->context(), *address)),
        cv);

    QObject::connect(dlg, &TableDialog::double_clicked, g_mainwindow,
                     [cv, dlg](const QModelIndex& index) {
                         auto* m = static_cast<ReferencesToModel*>(
                             dlg->source_model());
                         cv->surface()->jump_to(m->address(index));
                         dlg->accept();
                     });

    dlg->set_model(new ReferencesToModel(cv->context(), *address, dlg));
    dlg->set_stretch_last_column(false);
    dlg->resize_column(1, QHeaderView::Stretch);
    dlg->show();
}

void xrefs_edit() {
    auto sc = actions::resolve(Type::XREFS_EDIT);
    if(!sc || !sc->target.address.has_value()) return;

    ContextView* cv = sc->context_view;
    auto addr = sc->target.address;

    auto* dlgrefs = new ReferencesFromDialog(cv->context(), *addr, cv);

    QString str_addr = utils::to_hex(*addr, cv->context());
    dlgrefs->setWindowTitle(QString{"References from %1"}.arg(str_addr));

    QObject::connect(dlgrefs, &ReferencesFromDialog::accepted, cv, [cv]() {
        if(rd_reanalyze(cv->context())) {
            cv->schedule_step();
            cv->invalidate();
        }
    });

    dlgrefs->show();
}

void op_as_address() {
    auto sc = actions::resolve(Type::OP_AS_ADDRESS);
    if(!sc || !sc->target.row_address.has_value()) return;

    if(rd_operand_as_address(sc->context_view->context(),
                             *sc->target.row_address,
                             sc->target.cell->operand.index))
        sc->context_view->invalidate();
}

void op_as_immediate() {
    auto sc = actions::resolve(Type::OP_AS_IMMEDIATE);
    if(!sc || !sc->target.row_address.has_value()) return;

    if(rd_operand_as_immediate(sc->context_view->context(),
                               *sc->target.row_address,
                               sc->target.cell->operand.index))
        sc->context_view->invalidate();
}

void do_undefine() {
    auto sc = actions::resolve(Type::DO_UNDEFINE);
    if(!sc) return;

    ContextView* cv = sc->context_view;
    auto range = sc->target.range;

    if(range) {
        auto res =
            QMessageBox::question(g_mainwindow, "Please confirm",
                                  QString{"Do you want to undefine range %1-%2"}
                                      .arg(utils::to_hex(range->first))
                                      .arg(utils::to_hex(range->second)));

        if(res == QMessageBox::Yes &&
           rd_user_undefine_n(cv->context(), range->first,
                              range->second - range->first + 1,
                              RD_UNDEFINE_COMMENTS)) {
            cv->schedule_step();
            cv->surface()->clear_selection();
            cv->invalidate();
        }
    }
    else {
        auto address = sc->target.address;
        if(!address.has_value()) return;

        auto res =
            QMessageBox::question(g_mainwindow, "Please confirm",
                                  QString{"Do you want to undefine %1?"}.arg(
                                      utils::to_hex(*address)));

        if(res == QMessageBox::Yes &&
           rd_user_undefine(cv->context(), *address,
                            RD_UNDEFINE_TRACE | RD_UNDEFINE_COMMENTS)) {
            cv->schedule_step();
            cv->invalidate();
        }
    }
}

void do_code() {
    auto sc = actions::resolve(Type::DO_CODE);
    if(!sc || !sc->target.address.has_value()) return;

    ContextView* cv = sc->context_view;
    auto address = sc->target.address;

    auto res =
        QMessageBox::question(g_mainwindow, "Please confirm",
                              QString{"Do you want to convert to code %1?"}.arg(
                                  utils::to_hex(*address)));

    if(res != QMessageBox::Yes) return;

    if(rd_make_code(cv->context(), *address) && rd_reanalyze(cv->context())) {
        cv->schedule_step();
        cv->invalidate();
    }
}

void do_data() {
    auto sc = actions::resolve(Type::DO_DATA);
    if(!sc || !sc->target.address.has_value()) return;

    ContextView* cv = sc->context_view;
    auto address = sc->target.address;

    auto* dlgtypes = new TypesDialog(cv->context(), cv);

    QObject::connect(
        dlgtypes, &TypesDialog::accepted, cv,
        [cv, dlgtypes, addr = *address]() {
            bool ok = rd_user_type(cv->context(), addr,
                                   qUtf8Printable(dlgtypes->selected_type()),
                                   dlgtypes->selected_count(),
                                   dlgtypes->selected_modifier()) &&
                      rd_reanalyze(cv->context());

            if(ok) {
                cv->schedule_step();
                cv->invalidate();
            }
        });

    dlgtypes->show();
}

void do_branch() {
    auto sc = actions::resolve(Type::DO_BRANCH);
    if(!sc || !sc->target.address.has_value()) return;

    ContextView* cv = sc->context_view;
    auto address = sc->target.address;

    auto* dlgbranch = new BranchDialog(*address, cv);

    QObject::connect(
        dlgbranch, &BranchDialog::accepted, cv,
        [cv, dlgbranch, addr = *address]() {
            bool ok =
                rd_user_branch_table(
                    cv->context(), addr,
                    dlgbranch->selected_element_type().isEmpty()
                        ? nullptr
                        : qUtf8Printable(dlgbranch->selected_element_type()),
                    dlgbranch->selected_count(),
                    dlgbranch->selected_xref_type()) &&
                rd_reanalyze(cv->context());

            if(ok) {
                cv->schedule_step();
                cv->invalidate();
            }
        });

    dlgbranch->show();
}

void create_function() {
    auto sc = actions::resolve(Type::CREATE_FUNCTION);
    if(!sc || !sc->target.address.has_value()) return;

    ContextView* cv = sc->context_view;
    auto address = sc->target.address;

    bool ok = false;
    QString n = QInputDialog::getText(
        g_mainwindow,
        QString{"Create function @ %1"}.arg(utils::to_hex(*address)),
        "Function name (can be empty):", QLineEdit::Normal, {}, &ok);

    if(!ok) return;

    if(rd_set_function(cv->context(), *address) &&
       rd_reanalyze(cv->context())) {

        if(!n.isEmpty())
            rd_user_name(cv->context(), *address, qUtf8Printable(n));

        cv->schedule_step();
        cv->invalidate();
    }
}

void patch_instruction() {
    auto sc = actions::resolve(Type::PATCH_INSTRUCTION);
    if(!sc || !sc->target.address.has_value()) return;

    ContextView* cv = sc->context_view;
    auto address = sc->target.address;

    auto* dlgpatch = new PatchDialog(*address, cv);

    QObject::connect(dlgpatch, &PatchDialog::accepted, cv,
                     [cv, dlgpatch, addr = *address]() {
                         bool ok = rd_patch_instruction(
                             cv->context(), addr,
                             qUtf8Printable(dlgpatch->instruction_text()),
                             dlgpatch->fill_nop());

                         if(ok) {
                             cv->invalidate();
                             cv->schedule_step();
                         }
                     });

    dlgpatch->show();
}

void rename() {
    auto sc = actions::resolve(Type::RENAME);
    if(!sc || !sc->target.address.has_value()) return;

    ContextView* cv = sc->context_view;
    auto address = sc->target.address;

    QString name;

    if(const char* s = rd_get_name(cv->context(), *address); s) name = s;

    bool ok = false;
    QString s = QInputDialog::getText(
        g_mainwindow,
        QString("Rename @ %1").arg(rd_to_hexaddr(cv->context(), *address)),
        "New Name", QLineEdit::Normal, name, &ok);

    if(ok && rd_user_name(cv->context(), *address, qUtf8Printable(s)))
        cv->invalidate();
}

} // namespace

void init(QMainWindow* mw) {
    g_mainwindow = static_cast<MainWindow*>(mw);

    // clang-format off
    g_actiontable = {
        {Type::GOTO,              {IconKind::FA,   0xf1e5, "Goto",                 Qt::Key_G,                 show_goto,         {}}},
        {Type::COPY,              {IconKind::NONE, 0,      "Copy",                 Qt::CTRL | Qt::Key_C,      copy,              {}}},
        {Type::SELECT_ALL,        {IconKind::NONE, 0,      "Select All",           Qt::CTRL | Qt::Key_A,      select_all,        {}}},
        {Type::XREFS_TO,          {IconKind::NONE, 0,      "Cross References",     Qt::Key_X,                 xrefs_to,          {}}},
        {Type::XREFS_EDIT,        {IconKind::FA,   0xf074, "Edit References",      {},                        xrefs_edit,        {}}},
        {Type::RENAME,            {IconKind::NONE, 0,      "Rename",               Qt::Key_N,                 rename,            {}}},
        {Type::COMMENT,           {IconKind::NONE, 0,      "Comment",              Qt::Key_Semicolon,         comment,           {}}},
        {Type::OP_AS_ADDRESS,     {IconKind::NONE, 0,      "As Address",           Qt::Key_A,                 op_as_address,     {}}},
        {Type::OP_AS_IMMEDIATE,   {IconKind::NONE, 0,      "As Immediate",         Qt::Key_I,                 op_as_immediate,   {}}},
        {Type::DO_UNDEFINE,       {IconKind::FA,   0xf00d, "Undefine",             Qt::Key_U,                 do_undefine,       {}}},
        {Type::DO_CODE,           {IconKind::FA,   0xf121, "Code",                 Qt::Key_C,                 do_code,           {}}},
        {Type::DO_DATA,           {IconKind::FA,   0xf1b3, "Data",                 Qt::Key_D,                 do_data,           {}}},
        {Type::DO_BRANCH,         {IconKind::FA,   0xe4bc, "Branch Table",         {},                        do_branch,         {}}},
        {Type::CREATE_FUNCTION,   {IconKind::FA,   0x46,   "Create function",      Qt::Key_F,                 create_function,   {}}},
        {Type::PATCH_INSTRUCTION, {IconKind::FA,   0xf462, "Patch Instruction",    Qt::SHIFT | Qt::Key_Space, patch_instruction, {}}},
        {Type::REANALYZE,         {IconKind::NONE, 0,      "Reanalyze",            {},                        reanalyze,         {}}},
        {Type::REBASE,            {IconKind::NONE, 0,      "Rebase",               {},                        rebase,            {}}},
        {Type::SWITCH_TO_HEX,     {IconKind::FA,   0xe69b, "Hex Dump",             {},                        switch_hex,        {}}},
        {Type::SWITCH_TO_LISTING, {IconKind::FA,   0xf550, "Listing",              Qt::Key_Space,             switch_listing,    {}}},
        {Type::SWITCH_TO_GRAPH,   {IconKind::FA,   0xf542, "Graph",                Qt::Key_Space,             switch_graph,      {}}},
        {Type::OPEN_DETAILS,      {IconKind::FA,   0x3f,   "Details",              {},                        show_details,      {}}},
        {Type::OPEN_CALLGRAPH,    {IconKind::FA,   0xe695, "Call Graph",           {},                        show_callgraph,    {}}},
        {Type::OPEN_HOME,         {IconKind::FA,   0xf015, "Home",                 {},                        nullptr,           "https://redasm.dev"}},
        {Type::OPEN_GITHUB,       {IconKind::FAB,  0xf113, "Source Code",          {},                        nullptr,           "https://source.redasm.dev"}},
        {Type::OPEN_DISCORD,      {IconKind::FAB,  0xf392, "Discord",              {},                        nullptr,           "https://discord.redasm.dev"}},
        {Type::OPEN_X,            {IconKind::FAB,  0xe61b, "X",                    {},                        nullptr,           "https://x.com/re_dasm"}},
        {Type::OPEN_MASTODON,     {IconKind::FAB,  0xf4f6, "Infosec.exchange",     {},                        nullptr,           "https://infosec.exchange/@redasm"}},
        {Type::OPEN_FEEDBACK,     {IconKind::FA,   0xf188, "Send feedback",        {},                        nullptr,           "https://bugs.redasm.dev"}},
        {Type::OPEN_ABOUT,        {IconKind::FA,   0xf05a, "&About",               {},                        []() { (new AboutDialog(g_mainwindow))->show(); },    {}}},
        {Type::OPEN_SETTINGS,     {IconKind::FA,   0xf013, "&Settings",            {},                        []() { (new SettingsDialog(g_mainwindow))->show(); }, {}}},
    };
    // clang-format on
}

QAction* create(Type t, QWidget* parent, bool shortcut) {
    auto it = g_actiontable.constFind(t);
    if(it == g_actiontable.constEnd()) {
        qFatal() << "unhandled action type: " << t;
        return nullptr;
    }

    const ActionDesc& desc = it.value();

    QIcon icon;
    if(desc.icon_kind == IconKind::FA)
        icon = font_awesome::icon(QChar{desc.icon_id});
    else if(desc.icon_kind == IconKind::FAB)
        icon = font_awesome::brand(QChar{desc.icon_id});

    QKeySequence seq = shortcut && desc.key != QKeyCombination{}
                           ? QKeySequence{desc.key}
                           : QKeySequence{};

    auto openurl_slot = [url = desc.url]() {
        QDesktopServices::openUrl(QUrl{url});
    };

    QAction* act =
        !desc.url.isEmpty()
            ? parent->addAction(icon, desc.text, seq, parent, openurl_slot)
            : parent->addAction(icon, desc.text, seq, parent, desc.handler);

    act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    return act;
}

} // namespace actions
