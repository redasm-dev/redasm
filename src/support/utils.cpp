#include "utils.h"
#include "actions.h"
#include "support/fontawesome.h"
#include "support/themeprovider.h"
#include "views/surface/graph/graph.h"
#include "views/surface/hex.h"
#include "views/surface/listing.h"
#include <QAbstractItemModel>
#include <QClipboard>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScrollBar>
#include <QStackedWidget>
#include <QTimer>
#include <QToolButton>

namespace utils {

namespace {

constexpr int LOGO_SIZE = 64;
constexpr int LOGO_MARGIN_X = 10;
constexpr int LOGO_MARGIN_Y = 5;

QPixmap copy_screenshot(QWidget* w) {
    auto* stackw = qobject_cast<QStackedWidget*>(w);
    int crop_x = -1, crop_y = -1;

    if(stackw) { // Try to grab surfaces
        if(auto* l = stackw->findChild<SurfaceListing*>(); l && l->isVisible())
            w = l->viewport();
        else if(auto* g = stackw->findChild<SurfaceGraph*>();
                g && g->isVisible()) {
            w = g->viewport();
        }
        else if(auto* h = stackw->findChild<HexView*>(); h && h->isVisible()) {
            w = h->viewport();

            if(h->viewport()->verticalScrollBar()->isVisible())
                crop_x = h->viewport()->verticalScrollBar()->width();

            if(h->viewport()->horizontalScrollBar()->isVisible())
                crop_y = h->viewport()->horizontalScrollBar()->height();
        }
    }

    QPixmap logo = utils::get_logo();
    QPixmap scrshot =
        w->grab(QRect{0, 0, w->width() - crop_x, w->height() - crop_y});

    logo = logo.scaled(utils::LOGO_SIZE, utils::LOGO_SIZE, Qt::KeepAspectRatio,
                       Qt::SmoothTransformation);

    int x = scrshot.width() - logo.width() - utils::LOGO_MARGIN_X;
    int y = scrshot.height() - logo.height() - utils::LOGO_MARGIN_Y;

    QPainter painter(&scrshot);
    painter.setOpacity(0.5);
    painter.drawPixmap(x, y, logo);
    painter.end();

    return scrshot;
}

} // namespace

QString to_hex(RDAddress address, const RDContext* ctx) {
    QString s = QString::number(address, 16).toUpper();

    if(ctx) {
        return s.rightJustified(
            static_cast<qsizetype>(rd_get_ptr_size(ctx)) * 2, '0');
    }

    return s;
}

QString confidence_text(RDConfidence c) {
    switch(c) {
        case RD_CONFIDENCE_PLACEHOLDER: return "PLACEHOLDER";
        case RD_CONFIDENCE_AUTO: return "AUTO";
        case RD_CONFIDENCE_LIBRARY: return "LIBRARY";
        case RD_CONFIDENCE_USER: return "USER";
        default: break;
    }

    return {};
}

QMenu* create_surface_menu(ISurface* surface) {
    QWidget* w = surface->to_widget();
    auto* listing = qobject_cast<SurfaceListing*>(w);
    auto* graph = qobject_cast<SurfaceGraph*>(w);

    // clang-format off
    QAction* actcopy = actions::create(actions::COPY, w);
    QAction* actrefs = actions::create(actions::REFS_TO, w);
    QAction* actrename = actions::create(actions::RENAME, w);
    QAction* actcomment = actions::create(actions::COMMENT, w);
    QAction* act_op_as_addr = actions::create(actions::OP_AS_ADDRESS, w);
    QAction* act_op_as_imm = actions::create(actions::OP_AS_IMMEDIATE, w);
    QAction* act_undefine = actions::create(actions::DO_UNDEFINE, w);
    QAction* act_code = actions::create(actions::DO_CODE, w);
    QAction* act_data = actions::create(actions::DO_DATA, w);
    QAction* act_open_hex = actions::create(actions::SWITCH_TO_HEX, w);
    QAction* act_create_function = actions::create(actions::CREATE_FUNCTION, w);
    QAction* act_patch = actions::create(actions::PATCH_INSTRUCTION, w);
    // clang-format on

    auto* menu = new QMenu(w);
    menu->addAction(actcopy);
    menu->addAction(actrefs);
    menu->addAction(actrename);
    menu->addSeparator();
    menu->addAction(actcomment);
    menu->addAction(act_op_as_addr);
    menu->addAction(act_op_as_imm);
    menu->addAction(act_undefine);
    menu->addAction(act_code);
    menu->addAction(act_data);
    menu->addSeparator();

    if(listing) {
        QAction* act_switch_to_graph =
            actions::create(actions::SWITCH_TO_GRAPH, w);
        menu->addAction(act_switch_to_graph);
    }

    if(graph) {
        QAction* act_switch_to_listing =
            actions::create(actions::SWITCH_TO_LISTING, w);
        menu->addAction(act_switch_to_listing);
    }

    menu->addAction(act_open_hex);
    menu->addSeparator();
    menu->addAction(act_patch);
    menu->addAction(act_create_function);
    menu->addSeparator();
    menu->addAction(actions::create(actions::GOTO, w));
    menu->addSeparator();
    menu->addAction(actions::create(actions::OPEN_DETAILS, w));

    QObject::connect(menu, &QMenu::aboutToShow, w, [=]() {
        auto cursor_addr = surface->get_address_under_cursor();
        auto curr_addr = surface->get_current_address();

        if(cursor_addr.has_value()) {
            act_open_hex->setText(
                QString{"Hex Dump @ %1"}.arg(utils::to_hex(*cursor_addr)));
        }
        else if(curr_addr.has_value())
            act_open_hex->setText("Hex Dump");

        actcopy->setVisible(surface->has_selection());
        actrename->setVisible(cursor_addr.has_value());
        act_open_hex->setVisible(curr_addr.has_value());

        actrefs->setVisible(cursor_addr.has_value() &&
                            !rd_slice_is_empty(rd_get_xrefs_to(
                                surface->context(), *cursor_addr, RD_XR_NONE)));
        RDFlags f;
        bool flags_found = cursor_addr.has_value() &&
                           rd_get_flags(surface->context(), *cursor_addr, &f);

        if(flags_found) {
            const RDSegment* seg =
                rd_find_segment(surface->context(), *cursor_addr);
            act_create_function->setVisible(seg && (seg->perm & RD_SP_X) &&
                                            !rd_flags_has_func(f));
        }
        else
            act_create_function->setVisible(false);

        auto celldata = surface->get_cell_data_under_cursor();

        act_op_as_addr->setVisible(
            celldata && celldata->operand.index != -1 &&
            celldata->operand.value.kind == RD_OP_IMM &&
            rd_is_address(surface->context(), celldata->operand.value.imm));

        act_op_as_imm->setVisible(celldata && celldata->operand.index != -1 &&
                                  celldata->operand.value.kind == RD_OP_ADDR);

        flags_found = curr_addr.has_value() &&
                      rd_get_flags(surface->context(), *curr_addr, &f);

        if(flags_found) {
            const RDSegment* seg =
                rd_find_segment(surface->context(), *curr_addr);

            act_undefine->setVisible(seg && !rd_flags_has_unknown(f));
            act_data->setVisible(seg && !rd_flags_has_data(f));

            act_code->setVisible(seg && (seg->perm & RD_SP_X) &&
                                 !rd_flags_has_code(f));

            act_patch->setVisible(seg && (seg->perm & RD_SP_X) &&
                                  rd_flags_has_code(f));
        }
        else {
            act_undefine->setVisible(false);
            act_code->setVisible(false);
            act_data->setVisible(false);
            act_patch->setVisible(false);
        }
    });

    return menu;
}

FeedbackToolButton* create_screenshot_button(QWidget* w) {
    auto* tbfeedback = new FeedbackToolButton(w);
    tbfeedback->setIcon(FA_ICON(0xf030));

    QObject::connect(tbfeedback, &FeedbackToolButton::feedback, w, [w]() {
        QPixmap s = utils::copy_screenshot(w);
        if(s.isNull()) return;
        qApp->clipboard()->setPixmap(s);
    });

    return tbfeedback;
}

QPixmap get_about_logo() {
    if(theme_provider::is_dark_theme())
        return QPixmap{":/res/about_logo_dark.png"};

    return QPixmap{":/res/about_logo.png"};
}

QPixmap get_logo() {
    if(theme_provider::is_dark_theme()) return QPixmap{":/res/logo_dark.png"};
    return QPixmap{":/res/logo.png"};
}

bool handle_key_press(ISurface* surface, QKeyEvent* e) {
    RDSurfacePos pos = surface->get_position();
    auto [row, col] = pos;

    if(e->matches(QKeySequence::MoveToNextChar)) {
        surface->set_position(row, col + 1);
    }
    else if(e->matches(QKeySequence::MoveToPreviousChar)) {
        if(col > 0) surface->set_position(row, col - 1);
    }
    else if(e->matches(QKeySequence::MoveToStartOfLine)) {
        surface->set_position(row, 0);
    }
    else if(e->matches(QKeySequence::SelectNextChar)) {
        surface->select(row, col + 1);
    }
    else if(e->matches(QKeySequence::SelectPreviousChar)) {
        if(col > 0) surface->select(row, col - 1);
    }
    else if(e->matches(QKeySequence::SelectNextLine)) {
        surface->select(row + 1, col);
    }
    else if(e->matches(QKeySequence::SelectPreviousLine)) {
        if(row > 0) surface->select(row - 1, col);
    }
    else if(e->matches(QKeySequence::SelectStartOfLine)) {
        surface->select(row, 0);
    }
    else if(e->matches(QKeySequence::SelectStartOfDocument)) {
        surface->select(0, 0);
    }
    else
        return false;

    return true;
}

void configure_hex_input(QLineEdit* le) {
    const QRegularExpression H{"[a-fA-F0-9]*"};
    le->setValidator(new QRegularExpressionValidator(H, le));
    le->setMaxLength(sizeof(u64) * 2);
}

QString model_to_csv(const QAbstractItemModel* model, bool with_header) {
    QString csv;
    int rows = model->rowCount();
    int cols = model->columnCount();

    auto escape_field = [](const QString& field) -> QString {
        QString f = field;
        if(f.contains(',') || f.contains('"') || f.contains('\n')) {
            f.replace('"', "\"\"");
            return '"' + f + '"';
        }
        return f;
    };

    if(with_header) {
        QStringList header_fields;
        for(int c = 0; c < cols; ++c)
            header_fields << escape_field(
                model->headerData(c, Qt::Horizontal).toString());
        csv += header_fields.join(',') + '\n';
    }

    for(int r = 0; r < rows; ++r) {
        QStringList fields;

        for(int c = 0; c < cols; ++c) {
            QModelIndex idx = model->index(r, c);
            fields << escape_field(
                model->data(idx, Qt::DisplayRole).toString());
        }

        csv += fields.join(',') + '\n';
    }

    return csv;
}

} // namespace utils
