#include "utils.h"
#include "support/fontawesome.h"
#include "support/settings.h"
#include "support/themeprovider.h"
#include "views/surface/graph/view.h"
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
        if(auto* l = stackw->findChild<SurfaceListing*>();
           l && l->isVisible()) {
            w = l->viewport();
        }
        else if(auto* g = stackw->findChild<SurfaceGraphView*>();
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

    QPixmap scrshot =
        w->grab(QRect{0, 0, w->width() - crop_x, w->height() - crop_y});

    if(REDasmSettings{}.screenshot_logo()) {
        QPixmap logo = utils::get_logo();

        logo = logo.scaled(utils::LOGO_SIZE, utils::LOGO_SIZE,
                           Qt::KeepAspectRatio, Qt::SmoothTransformation);

        int x = scrshot.width() - logo.width() - utils::LOGO_MARGIN_X;
        int y = scrshot.height() - logo.height() - utils::LOGO_MARGIN_Y;

        QPainter painter(&scrshot);
        painter.setOpacity(0.5);
        painter.drawPixmap(x, y, logo);
        painter.end();
    }

    return scrshot;
}

} // namespace

QString to_hex(RDAddress address, const RDContext* ctx) {
    QString s = QString::number(static_cast<qulonglong>(address), 16).toUpper();

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
    else {
        return false;
    }

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

QString ref_typetostring(RDXRefType t) {
    switch(t) {
        case RD_DR_READ: return "READ";
        case RD_DR_WRITE: return "WRITE";
        case RD_DR_ADDRESS: return "ADDRESS";
        case RD_CR_CALL: return "CALL";
        case RD_CR_JUMP: return "JUMP";
        default: break;
    }

    return {};
}

QString ref_getdirection(RDAddress address, const RDXRef& r) {
    if(r.address > address) return "DOWN";
    if(r.address < address) return "UP";
    return "---";
}

} // namespace utils
