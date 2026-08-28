#include "typedefs.h"
#include "support/themeprovider.h"
#include <QFontDatabase>
#include <QTextCursor>
#include <algorithm>

namespace {

int _get_member_max_width(const RDParamSlice& members, RDContext* ctx) {
    int w = 0;

    const RDParam* m;
    rd_slice_each(m, members) {
        QString type_str = QString::fromUtf8(rd_type_to_str(&m->type, ctx));
        QString name_str = QString::fromUtf8(m->name);
        int curr_w = 3 + type_str.length() + 1 + name_str.length() + 1;
        w = std::max(curr_w, w);
    }

    return w;
}

void _generate_compound_tdef_code(QTextEdit* te, const RDTypeDef* tdef,
                                  RDContext* ctx) {
    QTextDocument* doc = te->document();
    QTextCursor cur(doc);

    bool is_union = rd_typedef_kind(tdef) == RD_TKIND_UNION;

    if(is_union) {
        cur.insertText(QString{"union %1 {\n"}.arg(
            QString::fromUtf8(rd_typedef_name(tdef))));
    }
    else {
        cur.insertText(QString{"struct %1 {\n"}.arg(
            QString::fromUtf8(rd_typedef_name(tdef))));
    }

    RDParamSlice members = rd_typedef_get_members(tdef);
    int max_w = is_union ? 0 : _get_member_max_width(members, ctx);

    const RDParam* m;
    rd_slice_each(m, members) {
        QString type_str = QString::fromUtf8(rd_type_to_str(&m->type, ctx));
        QString name_str = QString::fromUtf8(m->name);
        QString offset_str = QString::number(m->field_offset);

        QString code_part = QString{"   %1 %2;"}.arg(type_str).arg(name_str);

        if(!is_union) {
            int padding = max_w - code_part.length();
            cur.insertText(code_part + QString{padding, ' '} +
                           QString{"    // +%1"}.arg(offset_str));
        }

        cur.insertText("\n");
    }

    cur.insertText("}");
}

void _generate_function_tdef_code(QTextEdit* te, const RDTypeDef* tdef,
                                  RDContext* ctx) {
    QTextDocument* doc = te->document();
    QTextCursor cur(doc);

    const char* callconv = rd_typedef_get_callconv(tdef);
    if(callconv)
        cur.insertText(QString{"%1 "}.arg(QString::fromUtf8(callconv)));

    RDType ret;
    if(rd_typedef_get_ret(tdef, &ret)) {
        cur.insertText(
            QString{"%1 "}.arg(QString::fromUtf8(rd_type_to_str(&ret, ctx))));
    }

    if(rd_typedef_is_noret(tdef)) cur.insertText("noreturn ");

    cur.insertText(QString::fromUtf8(rd_typedef_name(tdef)));

    RDParamSlice args;
    if(rd_typedef_get_args(tdef, &args)) {
        cur.insertText("(\n");

        bool first = true;

        const RDParam* arg;
        rd_slice_each(arg, args) {
            if(!first) cur.insertText(",\n");
            first = false;

            QString arg_type =
                QString::fromUtf8(rd_type_to_str(&arg->type, ctx));
            QString arg_name = QString::fromUtf8(arg->name);
            cur.insertText(QString{"    %1 %2"}.arg(arg_type).arg(arg_name));
        }

        cur.insertText("\n)");
    }

    if(rd_typedef_is_proto(tdef)) cur.insertText("  // prototype");
}

} // namespace

TypeDefSyntaxHighlighter::TypeDefSyntaxHighlighter(QTextDocument* doc)
    : SyntaxHighlighter{doc} {
    m_typefmt.setForeground(theme_provider::color(RD_THEME_TYPE));

    this->set_number_fg(theme_provider::color(RD_THEME_NUMBER));
    this->set_line_comment_fg("//", theme_provider::color(RD_THEME_COMMENT));

    this->add_word_fg({"enum", "union", "struct"},
                      theme_provider::color(RD_THEME_CALL));

    this->add_word_fg({"noreturn"}, theme_provider::color(RD_THEME_CALL));

    this->add_type("void");
}

void TypeDefSyntaxHighlighter::add_type(const QString& type) {
    this->add_word_rule({type}, m_typefmt);
}

TypeDefsDialog::TypeDefsDialog(RDContext* ctx, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_context{ctx} {
    m_typedefsmodel = new TypedefsFilterModel(ctx, true, this);
    m_typedefsmodel->sort(0, Qt::AscendingOrder);

    auto* hl = new TypeDefSyntaxHighlighter(m_ui.tecode->document());
    RDTypeDefSlice tdefs = rd_get_all_type_defs(ctx);

    const RDTypeDef** tdef;
    rd_slice_each(tdef, tdefs) { hl->add_type(rd_typedef_name(*tdef)); }

    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_ui.tecode->setFont(f);
    m_ui.tvtypedefs->setModel(m_typedefsmodel);

    QHeaderView* hdrview = m_ui.tvtypedefs->header();
    hdrview->setSectionResizeMode(0, QHeaderView::Stretch);

    connect(m_ui.tvtypedefs, &QTreeView::clicked, this,
            &TypeDefsDialog::generate_code);
}

void TypeDefsDialog::generate_code(const QModelIndex& index) {
    const RDTypeDef* tdef = m_typedefsmodel->type_def(index);

    m_ui.tecode->clear();

    switch(rd_typedef_kind(tdef)) {
        case RD_TKIND_STRUCT:
        case RD_TKIND_UNION:
            _generate_compound_tdef_code(m_ui.tecode, tdef, m_context);
            break;

        case RD_TKIND_FUNC:
            _generate_function_tdef_code(m_ui.tecode, tdef, m_context);
            break;

        default: break;
    }
}
