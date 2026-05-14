#include "actions.h"
#include "dialogs/about.h"
#include "dialogs/detail.h"
#include "dialogs/goto.h"
#include "dialogs/settings.h"
#include "dialogs/table.h"
#include "mainwindow.h"
#include "models/references.h"
#include "support/fontawesome.h"
#include "support/utils.h"
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QHash>
#include <QInputDialog>

namespace actions {

namespace {

QHash<Type, QAction*> g_actions;
MainWindow* g_mainwindow;

const QString DETAIL_TEMPLATE = QString{R"(
    <b>Address:</b> %1<br>
    <b>Offset:</b> %2<br>
    <br>
    )"};

const QString INSTR_TEMPLATE = QString{R"(
        <b>==== INSTRUCTION ====</b><br>
        <b>Address:</b> %1<br>
        <b>Id:</b> %2<br>
        <b>Flow:</b> %3<br>
        <b>Length:</b> %4<br>
        <b>Delay slots:</b> %5<br>
        <br>
        )"};

const QString OP_TEMPLATE = QString{R"(
        <b>==== OPERAND %1 ====</b><br>
        <b>Type:</b> %2<br>
        <b>Userdata1:</b> %3<br>
        <b>Userdata2:</b> %4<br>
        )"};

QString optype_tostring(const RDOperand* op) {
    switch(op->kind) {
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
        [&, cv, dlggoto]() { cv->surface()->jump_to(dlggoto->address); });

    dlggoto->show();
}

void copy() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;
    qApp->clipboard()->setText(cv->surface()->get_selected_text());
}

void show_details() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;

    auto cursoraddr = cv->surface()->get_address_under_cursor();
    auto currentaddr = cv->surface()->get_current_address();
    RDAddress address;

    if(cursoraddr)
        address = *cursoraddr;
    else if(currentaddr)
        address = *currentaddr;
    else
        return;

    RDOffset offset;
    bool has_offset = rd_to_offset(cv->context(), address, &offset);

    QString s = DETAIL_TEMPLATE.arg(rd_to_hexaddr(cv->context(), address))
                    .arg(has_offset ? utils::to_hex_addr(offset) : "N/A");

    RDInstruction instr;

    if(rd_decode(cv->context(), address, &instr)) {
        QString strinstr =
            INSTR_TEMPLATE.arg(rd_to_hexaddr(cv->context(), instr.address))
                .arg(rd_to_hexaddr(cv->context(), instr.id))
                .arg(instrflow_tostring(&instr))
                .arg(rd_to_hexaddr(cv->context(), instr.length))
                .arg(QString::number(instr.delay_slots));

        rd_foreach_operand(i, op, &instr) {
            QString strop =
                OP_TEMPLATE.arg(i)
                    .arg(optype_tostring(op))
                    .arg(rd_to_hexaddr(cv->context(), op->userdata1))
                    .arg(rd_to_hexaddr(cv->context(), op->userdata2));

            switch(op->kind) {
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
                    strop.append(QString("<b>displ:</b> %1<br>")
                                     .arg(rd_to_hexaddr(cv->context(),
                                                        op->displ.offset)));
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

    RDXRefSlice xrefs = rd_get_xrefs_from(cv->context(), address);

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

    xrefs = rd_get_xrefs_to(cv->context(), address);

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

void comment() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;

    auto address = cv->surface()->get_current_address();
    if(!address) return;

    bool ok = false;
    const char* cmt = rd_get_comment(cv->context(), *address);

    QString s = QInputDialog::getMultiLineText(
        g_mainwindow, QString{"Comment @ %1"}.arg(utils::to_hex_addr(*address)),
        "Insert comment (or leave empty):", cmt ? cmt : QString{}, &ok);

    if(ok && rd_set_comment(cv->context(), *address, qUtf8Printable(s)))
        cv->surface()->invalidate();
}

void refs_to() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;

    auto address = cv->surface()->get_address_under_cursor();
    if(!address) return;

    auto* dlg = new TableDialog(
        QString("References to %1").arg(rd_to_hexaddr(cv->context(), *address)),
        cv);

    QObject::connect(dlg, &TableDialog::double_clicked, g_mainwindow,
                     [cv, dlg](const QModelIndex& index) {
                         auto* m = static_cast<ReferencesModel*>(dlg->model());
                         cv->surface()->jump_to(m->address(index));
                         dlg->accept();
                     });

    dlg->set_model(new ReferencesModel(cv->context(), *address, dlg));
    dlg->show();
}

void op_as_address() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;

    auto address = cv->surface()->get_current_address();
    if(!address) return;

    auto celldata = cv->surface()->get_cell_data_under_cursor();
    if(!celldata || celldata->operand.index == -1) return;

    const RDOperand& op = celldata->operand.value;
    if(op.kind != RD_OP_IMM || !rd_is_address(cv->context(), op.imm)) return;

    if(rd_operand_as_address(cv->context(), *address, celldata->operand.index))
        cv->surface()->invalidate();
}

void op_as_immediate() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;

    auto address = cv->surface()->get_current_address();
    if(!address) return;

    auto celldata = cv->surface()->get_cell_data_under_cursor();
    if(!celldata || celldata->operand.index == -1 ||
       celldata->operand.value.kind != RD_OP_ADDR)
        return;

    if(rd_operand_as_immediate(cv->context(), *address,
                               celldata->operand.index))
        cv->surface()->invalidate();
}

void rename() {
    ContextView* cv = g_mainwindow->context_view();
    if(!cv) return;

    auto address = cv->surface()->get_address_under_cursor();
    if(!address) return;

    QString name;

    if(const char* s = rd_get_name(cv->context(), *address); s) name = s;

    bool ok = false;
    QString s = QInputDialog::getText(
        g_mainwindow,
        QString("Rename @ %1").arg(rd_to_hexaddr(cv->context(), *address)),
        "New Name", QLineEdit::Normal, name, &ok);

    if(ok && rd_user_name(cv->context(), *address, qUtf8Printable(s)))
        cv->surface()->invalidate();
}

} // namespace

void init(QMainWindow* mw) {
    g_mainwindow = static_cast<MainWindow*>(mw);

    g_actions[Type::GOTO] =
        mw->addAction(FA_ICON(0xf1e5), "Goto…", QKeySequence{Qt::Key_G}, mw,
                      []() { actions::show_goto(); });

    g_actions[Type::COPY] =
        mw->addAction("Copy", QKeySequence{Qt::CTRL | Qt::Key_C}, mw,
                      []() { actions::copy(); });

    g_actions[Type::REFS_TO] =
        mw->addAction("Cross References To…", QKeySequence{Qt::Key_X}, mw,
                      []() { actions::refs_to(); });

    g_actions[Type::RENAME] = mw->addAction("Rename", QKeySequence(Qt::Key_N),
                                            mw, []() { actions::rename(); });

    g_actions[Type::COMMENT] =
        mw->addAction("Comment", QKeySequence{Qt::Key_Semicolon}, mw,
                      []() { actions::comment(); });

    g_actions[Type::OP_AS_ADDRESS] =
        mw->addAction("As Address", QKeySequence{Qt::Key_A}, mw,
                      []() { actions::op_as_address(); });

    g_actions[Type::OP_AS_IMMEDIATE] =
        mw->addAction("As Immediate", QKeySequence{Qt::Key_I}, mw,
                      []() { actions::op_as_immediate(); });

    g_actions[Type::OPEN_DETAILS] = mw->addAction(
        FA_ICON(0x3f), "Details", mw, []() { actions::show_details(); });

    g_actions[Type::OPEN_HOME] =
        mw->addAction(FA_ICON(0xf015), "Homepage", mw, []() {
            QDesktopServices::openUrl(QUrl{"https://redasm.dev"});
        });

    g_actions[Type::OPEN_TELEGRAM] =
        mw->addAction(FAB_ICON(0xf2c6), "Telegram", mw, []() {
            QDesktopServices::openUrl(QUrl{"https://t.me/REDasmDisassembler"});
        });

    g_actions[Type::OPEN_REDDIT] =
        mw->addAction(FAB_ICON(0xf281), "Reddit", mw, []() {
            QDesktopServices::openUrl(QUrl{"https://reddit.com/r/REDasm"});
        });

    g_actions[Type::OPEN_X] = mw->addAction(FAB_ICON(0xe61b), "X", mw, []() {
        QDesktopServices::openUrl(QUrl{"https://x.com/re_dasm"});
    });

    g_actions[Type::OPEN_GITHUB] =
        mw->addAction(FAB_ICON(0xf113), "Report an Issue", mw, []() {
            QDesktopServices::openUrl(
                QUrl{"https://github.com/REDasmOrg/REDasm/issues"});
        });

    g_actions[Type::OPEN_ABOUT] = mw->addAction("&About", mw, []() {
        auto* dlgabout = new AboutDialog(g_mainwindow);
        dlgabout->show();
    });

    g_actions[Type::OPEN_SETTINGS] = mw->addAction("&Settings", mw, []() {
        auto* dlgsettings = new SettingsDialog(g_mainwindow);
        dlgsettings->show();
    });
}

QAction* get(Type t) {
    if(auto it = g_actions.find(t); it != g_actions.end()) return it.value();
    return nullptr;
}

} // namespace actions
