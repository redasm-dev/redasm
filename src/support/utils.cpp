#include "utils.h"
#include "actions.h"
#include "support/fontawesome.h"
#include "support/themeprovider.h"
#include "views/surface/graph/graph.h"
#include "views/surface/listing.h"
#include <QClipboard>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStackedWidget>
#include <QTimer>
#include <QToolButton>

namespace utils {

namespace {

constexpr int LOGO_SIZE = 64;
constexpr int LOGO_MARGIN = 5;
constexpr int FEEDBACK_INTERVAL = 1000;
const char* const FEEDBACK_IN_PROGRESS = "__redasm_feedback__";

QPixmap copy_screenshot(QWidget* w) {
    auto* stackw = qobject_cast<QStackedWidget*>(w);

    if(stackw) { // Try to grab surfaces
        if(auto* l = stackw->findChild<SurfaceListing*>(); l && l->isVisible())
            w = l->viewport();
        else if(auto* g = stackw->findChild<SurfaceGraph*>();
                g && g->isVisible()) {
            w = g->viewport();
        }
    }

    QPixmap logo = utils::get_logo(), scrshot = w->grab();

    logo = logo.scaled(utils::LOGO_SIZE, utils::LOGO_SIZE, Qt::KeepAspectRatio,
                       Qt::SmoothTransformation);

    int x = scrshot.width() - logo.width() - utils::LOGO_MARGIN;
    int y = scrshot.height() - logo.height() - utils::LOGO_MARGIN;

    QPainter painter(&scrshot);
    painter.setOpacity(0.5);
    painter.drawPixmap(x, y, logo);
    painter.end();

    return scrshot;
}

void confirm_feedback(QToolButton* tb) {
    QIcon icon = tb->icon();
    tb->setProperty(utils::FEEDBACK_IN_PROGRESS, true);
    tb->setIcon(FA_ICON_COLOR(0xf00c, theme_provider::color(RD_THEME_SUCCESS)));

    QTimer::singleShot(utils::FEEDBACK_INTERVAL, [tb, icon]() {
        tb->setIcon(icon);
        tb->setProperty(utils::FEEDBACK_IN_PROGRESS, QVariant{});
    });
}

} // namespace

QString to_hex_addr(RDAddress address, const RDSegment* seg) {
    QString s = QString::number(address, 16).toUpper();
    if(seg) return s.rightJustified(static_cast<qsizetype>(seg->unit) * 2, '0');
    return s;
}

QString confidence_text(RDConfidence c) {
    switch(c) {
        case RD_CONFIDENCE_AUTO: return "AUTO";
        case RD_CONFIDENCE_LIBRARY: return "LIBRARY";
        case RD_CONFIDENCE_USER: return "USER";
        default: break;
    }

    return {};
}

QMenu* create_surface_menu(ISurface* surface) {
    QAction* actcopy = actions::get(actions::COPY);
    QAction* actrefs = actions::get(actions::REFS_TO);
    QAction* actrename = actions::get(actions::RENAME);
    QAction* actcomment = actions::get(actions::COMMENT);
    QAction* act_op_as_addr = actions::get(actions::OP_AS_ADDRESS);
    QAction* act_op_as_imm = actions::get(actions::OP_AS_IMMEDIATE);

    auto* menu = new QMenu(surface->to_widget());
    menu->addAction(actcopy);
    menu->addAction(actrefs);
    menu->addAction(actrename);
    menu->addSeparator();
    menu->addAction(actcomment);
    menu->addAction(act_op_as_addr);
    menu->addAction(act_op_as_imm);
    menu->addSeparator();
    menu->addAction(actions::get(actions::GOTO));
    menu->addSeparator();
    menu->addAction(actions::get(actions::OPEN_DETAILS));

    QObject::connect(menu, &QMenu::aboutToShow, surface->to_widget(), [=]() {
        auto cursoraddr = surface->get_address_under_cursor();

        actcopy->setVisible(surface->has_selection());
        actrename->setVisible(cursoraddr.has_value());

        actrefs->setVisible(cursoraddr.has_value() &&
                            !rd_slice_is_empty(rd_get_xrefs_to(
                                surface->context(), *cursoraddr)));

        auto celldata = surface->get_cell_data_under_cursor();

        act_op_as_addr->setVisible(
            celldata && celldata->operand.index != -1 &&
            celldata->operand.value.kind == RD_OP_IMM &&
            rd_is_address(surface->context(), celldata->operand.value.imm));

        act_op_as_imm->setVisible(celldata && celldata->operand.index != -1 &&
                                  celldata->operand.value.kind == RD_OP_ADDR);
    });

    return menu;
}

QToolButton* create_screenshot_button(QWidget* w) {
    auto* pb = new QToolButton(w);
    pb->setIcon(FA_ICON(0xf030));

    QObject::connect(pb, &QToolButton::clicked, w, [w, pb]() {
        if(!pb->property(utils::FEEDBACK_IN_PROGRESS).isNull()) return;

        QPixmap s = utils::copy_screenshot(w);
        if(s.isNull()) return;

        qApp->clipboard()->setPixmap(s);
        utils::confirm_feedback(pb);
    });

    return pb;
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
    else if(e->matches(QKeySequence::MoveToNextLine)) {
        surface->set_position(row + 1, col);
    }
    else if(e->matches(QKeySequence::MoveToPreviousLine)) {
        if(row > 0) surface->set_position(row - 1, col);
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

} // namespace utils
