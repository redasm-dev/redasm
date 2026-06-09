#include "memorymap.h"
#include "hexview/flagsbuffer.h"
#include "support/surfacerenderer.h"
#include "support/utils.h"

MemoryMapDialog::MemoryMapDialog(const RDContext* ctx, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_context{ctx} {

    m_flagsdelegate = new FlagsDelegate(m_ui.hexview);

    m_ui.hexview->setFont(surface_renderer::get_font());
    m_ui.hexview->setDelegate(m_flagsdelegate);

    RDSegmentSlice segments = rd_get_all_segments(ctx);

    for(usize i = 0; i < rd_slice_length(segments); i++) {
        const RDSegment* s = rd_slice_at(segments, i);
        QString startaddr = utils::to_hex(s->start_address, s);
        QString endaddr = utils::to_hex(s->end_address, s);

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
    m_flagsdelegate->set_flags_buffer(flagsbuffer->flags());

    QHexDocument* hexdocument = QHexDocument::fromBuffer(flagsbuffer);
    m_ui.hexview->setDocument(hexdocument);
    m_ui.hexview->setBaseAddress(flagsbuffer->base_address());

    if(olddoc) olddoc->deleteLater();
}
