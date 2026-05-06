#include "statusbar.h"
#include "support/fontawesome.h"
#include "support/themeprovider.h"
#include "support/utils.h"
#include <redasm/redasm.h>

namespace statusbar {

namespace {

QLabel* g_lblstatuslabel;
QPushButton* g_pbstatus;
QPushButton* g_pbproblems;
bool g_busy = false;

} // namespace

QLabel* create_status_label(QWidget* parent) {
    g_lblstatuslabel = new QLabel(parent);
    return g_lblstatuslabel;
};

QPushButton* create_problems_button(int size, QWidget* parent) {
    g_pbproblems = new QPushButton(parent);
    g_pbproblems->setFlat(true);
    g_pbproblems->setText("\uf071");
    g_pbproblems->setFont(font_awesome::fa_font());
    g_pbproblems->setFixedSize(size, size);
    g_pbproblems->hide();
    return g_pbproblems;
}

QPushButton* create_status_button(int size, QWidget* parent) {
    g_pbstatus = new QPushButton(parent);
    g_pbstatus->setFlat(true);
    g_pbstatus->setText("\uf0c8");
    g_pbstatus->setFont(font_awesome::fa_font());
    g_pbstatus->setFixedSize(size, size);
    g_pbstatus->hide();
    return g_pbstatus;
}

void set_status_text(const QString& s) { g_lblstatuslabel->setText(s); }

void set_address(ISurface* surface) {
    if(g_busy) return;

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
        QString{"color: %1;"}.arg(theme_provider::color(RD_THEME_FAIL).name());

    g_pbstatus->setStyleSheet(STYLE);
    g_pbstatus->show();
    g_busy = true;
}

void set_pause_status() {
    static const QString STYLE = QString{"color: %1;"}.arg(
        theme_provider::color(RD_THEME_WARNING).name());

    g_pbstatus->setStyleSheet(STYLE);
    g_pbstatus->show();
    g_busy = false;
}

void set_ready_status() {
    static const QString STYLE = QString{"color: %1;"}.arg(
        theme_provider::color(RD_THEME_SUCCESS).name());

    g_pbstatus->setStyleSheet(STYLE);
    g_pbstatus->show();
    g_busy = false;
}

void check_problems(const RDContext* ctx) {
    if(!ctx) {
        g_pbproblems->hide();
        return;
    }

    static const QString STYLE = QString{"color: %1;"}.arg(
        theme_provider::color(RD_THEME_WARNING).name());

    RDProblemSlice problems = rd_get_all_problems(ctx);
    if(rd_slice_is_empty(problems)) return;

    g_pbproblems->setStyleSheet(STYLE);
    g_pbproblems->show();
}

QPushButton* problems_button() { return g_pbproblems; }
QPushButton* status_button() { return g_pbstatus; }

} // namespace statusbar
