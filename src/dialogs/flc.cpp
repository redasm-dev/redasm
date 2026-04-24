#include "flc.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>

FLCDialog::FLCDialog(RDContext* ctx, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_context{ctx} {
    const QRegularExpression H("[a-fA-F0-9]*");
    m_ui.leaddress->setValidator(new QRegularExpressionValidator(H, this));
    m_ui.leoffset->setValidator(new QRegularExpressionValidator(H, this));

    connect(m_ui.leaddress, &QLineEdit::textChanged, this,
            &FLCDialog::on_address_changed);

    connect(m_ui.leoffset, &QLineEdit::textChanged, this,
            &FLCDialog::on_offset_changed);

    connect(m_ui.lesegment, &QLineEdit::textChanged, this,
            [&](const QString& s) {
                m_ui.pbcopysegment->setEnabled(!s.isEmpty());
            });
}

void FLCDialog::on_address_changed(const QString& s) {
    if(s.isEmpty()) {
        m_ui.pbcopyaddress->setEnabled(false);
        return;
    }

    bool ok = true;

    if(m_ui.leaddress->hasFocus()) {
        usize val = s.toULongLong(&ok, 16);

        if(ok) {
            RDOffset offset{};

            if(rd_to_offset(m_context, val, &offset)) {
                m_ui.leoffset->setText(rd_to_hex(m_context, offset));

                const RDSegment* seg = rd_find_segment(m_context, val);
                if(seg)
                    m_ui.lesegment->setText(seg->name);
                else
                    m_ui.lesegment->clear();
            }
            else {
                m_ui.leoffset->clear();
                m_ui.lesegment->clear();
            }
        }
    }

    m_ui.pbcopyaddress->setEnabled(ok);
}

void FLCDialog::on_offset_changed(const QString& s) {
    if(s.isEmpty()) {
        m_ui.pbcopyoffset->setEnabled(false);
        return;
    }

    bool ok = true;

    if(m_ui.leoffset->hasFocus()) {
        usize val = s.toULongLong(&ok, 16);

        if(ok) {
            RDAddress address{};

            if(rd_to_address(m_context, val, &address)) {
                m_ui.leaddress->setText(rd_to_hex(m_context, address));

                const RDSegment* seg = rd_find_segment(m_context, address);

                if(seg)
                    m_ui.lesegment->setText(seg->name);
                else
                    m_ui.lesegment->clear();
            }
            else {
                m_ui.leaddress->clear();
                m_ui.lesegment->clear();
            }
        }
    }

    m_ui.pbcopyoffset->setEnabled(ok);
}
