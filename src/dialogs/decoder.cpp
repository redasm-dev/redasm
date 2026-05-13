#include "decoder.h"
#include "support/themeprovider.h"
#include "support/utils.h"
#include <QFontDatabase>
#include <QHexView/model/buffer/qmemorybuffer.h>
#include <redasm/redasm.h>

DecoderDialog::DecoderDialog(QWidget* parent): QDialog{parent}, m_ui{this} {
    utils::configure_hex_input(m_ui.leaddress);
    this->populate_processors();
    this->check_decode();

    // insert mode by default
    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_ui.ptedecoded->setFont(f);

    m_ui.leaddress->setText("0");
    m_ui.hexview->setCursorMode(QHexCursor::Mode::Insert);

    connect(m_ui.cbprocessors, &QComboBox::currentIndexChanged, this,
            &DecoderDialog::check_decode);

    connect(m_ui.leaddress, &QLineEdit::textChanged, this,
            [&](const QString& s) {
                if(s.isEmpty())
                    m_ui.hexview->setBaseAddress(0);
                else
                    m_ui.hexview->setBaseAddress(s.toULongLong(nullptr, 16));

                this->check_decode();
            });

    connect(m_ui.hexview, &QHexView::dataChanged, this,
            &DecoderDialog::check_decode);

    connect(m_ui.hexview, &QHexView::dataChanged, this,
            &DecoderDialog::check_decode);

    connect(m_ui.pbclear, &QPushButton::clicked, this, [&]() {
        QHexDocument* olddocument = m_ui.hexview->hexDocument();
        QHexDocument* newdocument =
            QHexDocument::fromMemory<QMemoryBuffer>(QByteArray{}, m_ui.hexview);
        m_ui.hexview->setDocument(newdocument);
        if(olddocument) olddocument->deleteLater();
        m_ui.leaddress->setText("0");
        this->check_decode();
    });

    connect(m_ui.buttonbox, &QDialogButtonBox::accepted, this,
            &DecoderDialog::do_decode);
}

void DecoderDialog::populate_processors() const {
    RDPluginSlice plugins = rd_get_all_processor_plugins();
    m_ui.cbprocessors->addItem(QString{}); // empty placeholder

    const RDPlugin** it;
    rd_slice_each(it, plugins) {
        m_ui.cbprocessors->addItem((*it)->processor->name,
                                   (*it)->processor->id);
    }
}

void DecoderDialog::check_decode() { // NOLINT
    bool can_decode = !m_ui.leaddress->text().isEmpty() &&
                      m_ui.cbprocessors->currentIndex() > 0 &&
                      !m_ui.hexview->hexDocument()->isEmpty();

    m_ui.buttonbox->button(QDialogButtonBox::Ok)->setEnabled(can_decode);
}

void DecoderDialog::do_decode() { // NOLINT
    m_ui.ptedecoded->clear();

    const QHexDocument* doc = m_ui.hexview->hexDocument();
    QByteArray ba = doc->read(0, doc->length());
    const char* p = ba.data();
    auto len = static_cast<usize>(ba.length());

    QString straddr = m_ui.leaddress->text();
    auto addr = static_cast<RDAddress>(straddr.toULongLong(nullptr, 16));

    RDPluginSlice plugins = rd_get_all_processor_plugins();
    RDDecodedInstruction dec;

    m_ui.ptedecoded->clear();
    QTextCursor cursor = m_ui.ptedecoded->textCursor();
    cursor.movePosition(QTextCursor::End);

    while(rd_decode_bytes(
        &p, &len, &addr,
        rd_slice_at(plugins, m_ui.cbprocessors->currentIndex() - 1)->processor,
        &dec)) {

        if(!m_ui.ptedecoded->document()->isEmpty()) cursor.insertBlock();

        QString addrstr = QString::number(dec.instr.address, 16)
                              .rightJustified(sizeof(u32) * 2, '0');

        QTextCharFormat cf;
        cursor.insertText(addrstr, cf);
        cursor.insertText(" ", cf);
        cursor.insertText(QString::fromUtf8(dec.instr_text), cf);

        if(m_ui.cbxdetails->isChecked()) {
            cursor.insertBlock();
            cf.setForeground(theme_provider::color(RD_THEME_COMMENT));

            // indent lines
            QStringList dump =
                QString::fromUtf8(rd_dump_instruction(&dec.instr)).split("\n");

            for(const QString& s : dump) {
                if(s != dump.front()) cursor.insertBlock();
                cursor.insertText("  ", cf);
                cursor.insertText(s, cf);
            }
        }
    }
}
