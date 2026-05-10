#include "segmentregs.h"
#include <QHeaderView>

SegmentRegsDialog::SegmentRegsDialog(RDContext* ctx, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_context{ctx} {

    m_model = new SegmentRegsModel(ctx, this);

    m_ui.tvtable->setModel(m_model);
    m_ui.tvtable->header()->setStretchLastSection(false);
    m_ui.tvtable->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_ui.tvtable->header()->setSectionResizeMode(2, QHeaderView::Stretch);

    this->fetch_registers();

    connect(m_ui.cbregisters, &QComboBox::currentIndexChanged, this,
            &SegmentRegsDialog::show_register);
}

void SegmentRegsDialog::fetch_registers() {
    m_ui.cbregisters->clear();

    RDSegmentRegNameSlice sregnames = rd_get_all_sreg_names(m_context);

    if(rd_slice_is_empty(sregnames)) {
        m_ui.cbregisters->hide();
        return;
    }

    const char** n;
    rd_slice_each(n, sregnames) {
        m_ui.cbregisters->addItem(QString::fromUtf8(*n));
    }

    m_ui.cbregisters->show();
    this->show_register(m_ui.cbregisters->currentIndex());
}

void SegmentRegsDialog::show_register(int idx) {
    RDSegmentRegNameSlice sregnames = rd_get_all_sreg_names(m_context);

    if(idx < static_cast<int>(rd_slice_length(sregnames)))
        m_model->select_register(rd_slice_at(sregnames, idx));
    else
        m_model->select_register(nullptr);
}
