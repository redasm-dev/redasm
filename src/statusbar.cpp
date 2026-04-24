#include "statusbar.h"
#include "mainwindow.h"
#include "support/fontawesome.h"
#include "support/themeprovider.h"
#include "support/utils.h"
#include <redasm/redasm.h>

namespace statusbar {

namespace {

const QString LISTING_MODE_TEXT = "Listing";
const QString RDIL_MODE_TEXT = "RDIL";

QLabel* g_lblstatuslabel;
QPushButton* g_pbstatus;
QPushButton* g_pbproblems;
QPushButton* g_pbrdiltoggle;

} // namespace

QLabel* create_status_label(QWidget* parent) {
    g_lblstatuslabel = new QLabel(parent);
    return g_lblstatuslabel;
};

QPushButton* create_problems_button(int size, QWidget* parent) {
    g_pbproblems = new QPushButton(parent);
    g_pbproblems->setFlat(true);
    g_pbproblems->setText("\uf071");
    g_pbproblems->setFont(fontawesome::fa_font());
    g_pbproblems->setFixedSize(size, size);
    g_pbproblems->hide();
    return g_pbproblems;
}

QPushButton* create_status_button(int size, QWidget* parent) {
    g_pbstatus = new QPushButton(parent);
    g_pbstatus->setFlat(true);
    g_pbstatus->setText("\uf0c8");
    g_pbstatus->setFont(fontawesome::fa_font());
    g_pbstatus->setFixedSize(size, size);
    g_pbstatus->hide();
    return g_pbstatus;
}

QPushButton* create_rdil_toggle_button(int size, QWidget* parent) {
    g_pbrdiltoggle = new QPushButton(parent);
    g_pbrdiltoggle->setFlat(true);
    g_pbrdiltoggle->setFixedHeight(size);
    g_pbrdiltoggle->hide();

    QObject::connect(statusbar::rdil_toggle_button(), &QPushButton::clicked,
                     parent, []() { statusbar::toggle_rdil(); });

    return g_pbrdiltoggle;
}

void set_status_text(const QString& s) { g_lblstatuslabel->setText(s); }

void set_address(ISurface* surface) {
    auto address = surface->get_current_address();

    if(!address) {
        g_lblstatuslabel->clear();
        return;
    }

    RDContext* ctx = surface->context();
    const RDFunction* f = rd_find_function(ctx, *address);
    QString s;

    if(f) {
        RDAddress funcaddr = rd_function_get_address(f);
        s += QString{"<b>Function: </b>%1"}.arg(rd_get_name(ctx, funcaddr));

        if(funcaddr != *address) {
            int diff = qMax(funcaddr, *address) - qMin(funcaddr, *address);

            if(*address < funcaddr)
                s += "-";
            else
                s += "+";

            s += QString::number(diff, 16);
        }

        s += QString::fromWCharArray(L"\u00A0\u00A0");
    }

    const RDSegment* seg = rd_find_segment(ctx, *address);

    s += QString::fromWCharArray(L"<b>Address: </b>%1\u00A0\u00A0")
             .arg(utils::to_hex_addr(*address, seg));

    RDOffset offset;

    if(rd_to_offset(ctx, *address, &offset)) {
        s += QString::fromWCharArray(L"<b>Offset: </b>%1\u00A0\u00A0")
                 .arg(utils::to_hex_addr(*address, seg));
    }

    if(seg) {
        s += QString::fromWCharArray(L"<b>Segment: </b>%1\u00A0\u00A0")
                 .arg(seg->name);
    }

    g_lblstatuslabel->setText(s);
}

void set_busy_status() {
    static const QString STYLE =
        QString{"color: %1;"}.arg(themeprovider::color(RD_THEME_FAIL).name());

    g_pbstatus->setStyleSheet(STYLE);
    g_pbstatus->show();
}

void set_pause_status() {
    static const QString STYLE = QString{"color: %1;"}.arg(
        themeprovider::color(RD_THEME_WARNING).name());

    g_pbstatus->setStyleSheet(STYLE);
    g_pbstatus->show();
}

void set_ready_status() {
    static const QString STYLE = QString{"color: %1;"}.arg(
        themeprovider::color(RD_THEME_SUCCESS).name());

    g_pbstatus->setStyleSheet(STYLE);
    g_pbstatus->show();
}

void toggle_rdil() {
    ContextView* cv = utils::mainwindow->context_view();
    if(!cv) return;

    ISurface* s = cv->surface();
    if(!s) return;

    s->set_rdil(!s->has_rdil());
    s->to_widget()->setFocus();
    statusbar::check_rdil();
}

void check_rdil() {
    ContextView* cv = utils::mainwindow->context_view();

    if(!cv) {
        g_pbrdiltoggle->hide();
        return;
    }

    ISurface* s = cv->surface();

    if(s) {
        g_pbrdiltoggle->setText(s->has_rdil() ? RDIL_MODE_TEXT
                                              : LISTING_MODE_TEXT);
        g_pbrdiltoggle->show();
    }
    else
        g_pbrdiltoggle->hide();
}

void check_problems(const RDContext* ctx) {
    if(!ctx) {
        g_pbproblems->hide();
        return;
    }

    static const QString STYLE = QString{"color: %1;"}.arg(
        themeprovider::color(RD_THEME_WARNING).name());

    RDProblemSlice problems = rd_get_all_problems(ctx);
    if(rd_slice_is_empty(problems)) return;

    g_pbproblems->setStyleSheet(STYLE);
    g_pbproblems->show();
}

QPushButton* problems_button() { return g_pbproblems; }
QPushButton* status_button() { return g_pbstatus; }
QPushButton* rdil_toggle_button() { return g_pbrdiltoggle; }

} // namespace statusbar
