#include "memorymap.h"
#include "hexview/flagsbuffer.h"
#include "support/utils.h"

MemoryMapDialog::MemoryMapDialog(const RDContext* ctx, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_context{ctx} {

    RDSegmentSlice segments = rd_get_all_segments(ctx);

    for(usize i = 0; i < rd_slice_length(segments); i++) {
        const RDSegment* s = rd_slice_at(segments, i);
        QString startaddr = utils::to_hex_addr(s->start_address, s);
        QString endaddr = utils::to_hex_addr(s->end_address, s);

        m_ui.cbsegments->addItem(
            QString{"%1 (%2 - %3)"}.arg(s->name).arg(startaddr).arg(endaddr));
    }

    this->show_memory(m_ui.cbsegments->currentIndex());

    connect(m_ui.cbsegments, &QComboBox::currentIndexChanged, this,
            &MemoryMapDialog::show_memory);
}

void MemoryMapDialog::show_memory(int idx) {
    RDSegmentSlice segments = rd_get_all_segments(m_context);
    const RDSegment* s = rd_slice_at(segments, idx);

    QHexDocument* olddoc = m_ui.hexview->hexDocument();

    auto* flagsbuffer = new FlagsBuffer(s);
    QHexDocument* hexdocument = QHexDocument::fromBuffer(flagsbuffer);
    m_ui.hexview->setDocument(hexdocument);
    m_ui.hexview->setBaseAddress(flagsbuffer->base_address());

    if(olddoc) olddoc->deleteLater();
}
