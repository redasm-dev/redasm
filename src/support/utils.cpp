#include "utils.h"
#include "actions.h"
#include "support/fontawesome.h"
#include "support/themeprovider.h"
#include "views/surface/graph/graph.h"
#include "views/surface/listing.h"
#include <QAbstractItemModel>
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

} // namespace

QString to_hex(RDAddress address, const RDSegment* seg) {
    QString s = QString::number(address, 16).toUpper();
    if(seg) return s.rightJustified(static_cast<qsizetype>(seg->unit) * 2, '0');
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
    QAction* actcopy = actions::get(actions::COPY);
    QAction* actrefs = actions::get(actions::REFS_TO);
    QAction* actrename = actions::get(actions::RENAME);
    QAction* actcomment = actions::get(actions::COMMENT);
    QAction* act_op_as_addr = actions::get(actions::OP_AS_ADDRESS);
    QAction* act_op_as_imm = actions::get(actions::OP_AS_IMMEDIATE);
    QAction* act_patch = actions::get(actions::PATCH_INSTRUCTION);

    auto* menu = new QMenu(surface->to_widget());
    menu->addAction(actcopy);
    menu->addAction(actrefs);
    menu->addAction(actrename);
    menu->addSeparator();
    menu->addAction(actcomment);
    menu->addAction(act_op_as_addr);
    menu->addAction(act_op_as_imm);
    menu->addAction(act_patch);
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
                                surface->context(), *cursoraddr, RD_XR_NONE)));

        auto celldata = surface->get_cell_data_under_cursor();

        act_op_as_addr->setVisible(
            celldata && celldata->operand.index != -1 &&
            celldata->operand.value.kind == RD_OP_IMM &&
            rd_is_address(surface->context(), celldata->operand.value.imm));

        act_op_as_imm->setVisible(celldata && celldata->operand.index != -1 &&
                                  celldata->operand.value.kind == RD_OP_ADDR);

        act_patch->setVisible(celldata && celldata->is_instruction);
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
