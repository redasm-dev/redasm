#include "flc.h"
#include "support/utils.h"
#include <QClipboard>
#include <QGuiApplication>

namespace {

std::optional<u64> parse_hex(const QString& s) {
    bool ok = false;
    u64 v = s.toULongLong(&ok, 16);
    return ok ? std::optional<u64>{v} : std::nullopt;
}

} // namespace

FLCDialog::FLCDialog(RDContext* ctx, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_context{ctx} {
    utils::configure_hex_input(m_ui.leaddress);
    utils::configure_hex_input(m_ui.lereladdress);
    utils::configure_hex_input(m_ui.leoffset);

    connect(m_ui.leaddress, &QLineEdit::textEdited, this,
            &FLCDialog::on_address_edited);

    connect(m_ui.lereladdress, &QLineEdit::textEdited, this,
            &FLCDialog::on_rel_address_edited);

    connect(m_ui.leoffset, &QLineEdit::textEdited, this,
            &FLCDialog::on_offset_edited);

    auto bind_copy = [this](QLineEdit* le, FeedbackPushButton* pb) {
        connect(le, &QLineEdit::textChanged, this,
                [pb](const QString& s) { pb->setEnabled(!s.isEmpty()); });

        connect(pb, &FeedbackPushButton::feedback, this,
                [le]() { QGuiApplication::clipboard()->setText(le->text()); });
    };

    bind_copy(m_ui.leaddress, m_ui.pbcopyaddress);
    bind_copy(m_ui.lereladdress, m_ui.pbcopyreladdress);
    bind_copy(m_ui.leoffset, m_ui.pbcopyoffset);
    bind_copy(m_ui.lesegment, m_ui.pbcopysegment);
}

void FLCDialog::on_address_edited(const QString& s) {
    this->update_from(m_ui.leaddress, parse_hex(s));
}

void FLCDialog::on_rel_address_edited(const QString& s) {
    std::optional<RDAddress> address;

    if(auto rel = parse_hex(s)) {
        RDAddress a;
        if(rd_to_absolute(m_context, *rel, &a)) address = a;
    }

    this->update_from(m_ui.lereladdress, address);
}

void FLCDialog::on_offset_edited(const QString& s) {
    std::optional<RDAddress> address;

    if(auto off = parse_hex(s)) {
        RDAddress a;
        if(rd_to_address(m_context, *off, &a)) address = a;
    }

    this->update_from(m_ui.leoffset, address);
}

void FLCDialog::update_from(const QLineEdit* source,
                            const std::optional<RDAddress>& address) {
    std::optional<u64> rel, offset;
    const RDSegment* seg = nullptr;

    if(address) {
        RDRelAddress r;
        if(rd_to_relative(m_context, *address, &r)) rel = r;

        RDOffset off;
        if(rd_to_offset(m_context, *address, &off)) offset = off;

        seg = rd_segment_find(m_context, *address);
    }

    auto set = [&](QLineEdit* le, const std::optional<u64>& v) {
        if(le == source) return;

        if(v)
            le->setText(QString::fromUtf8(rd_to_hexaddr(m_context, *v)));
        else
            le->clear();
    };

    set(m_ui.leaddress, address);
    set(m_ui.lereladdress, rel);
    set(m_ui.leoffset, offset);

    if(seg)
        m_ui.lesegment->setText(QString::fromUtf8(rd_segment_get_name(seg)));
    else
        m_ui.lesegment->clear();
}
