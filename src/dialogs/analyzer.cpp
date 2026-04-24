#include "analyzer.h"

AnalyzerDialog::AnalyzerDialog(RDContext* ctx, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_context{ctx} {
    m_analyzersmodel = new QStandardItemModel(m_ui.tblanalyzers);
    m_ui.tblanalyzers->setModel(m_analyzersmodel);

    this->get_analyzers();
    this->set_details_visible(false);

    connect(m_analyzersmodel, &QStandardItemModel::itemChanged, this,
            [&](QStandardItem* item) {
                rd_analyzeritem_select(
                    rd_slice_at(m_analyzers, item->index().row()),
                    item->checkState() == Qt::Checked);
            });

    connect(m_ui.chkshowdetails, &QCheckBox::checkStateChanged, this,
            [&](Qt::CheckState state) {
                this->set_details_visible(state == Qt::Checked);
            });

    connect(m_ui.pbselectall, &QPushButton::clicked, this,
            [&]() { this->select_analyzers(true); });
    connect(m_ui.pbunselectall, &QPushButton::clicked, this,
            [&]() { this->select_analyzers(false); });
    connect(m_ui.pbrestoredefaults, &QPushButton::clicked, this,
            &AnalyzerDialog::get_analyzers);
}

void AnalyzerDialog::set_details_visible(bool b) {
    m_ui.tblanalyzers->setColumnHidden(m_analyzersmodel->columnCount() - 1, !b);
    m_ui.tblanalyzers->horizontalHeader()->setVisible(b);
}

void AnalyzerDialog::select_analyzers(bool select) {
    for(int i = 0; i < m_analyzersmodel->rowCount(); i++) {
        auto* item = m_analyzersmodel->item(i);
        item->setCheckState(select ? Qt::Checked : Qt::Unchecked);
        rd_analyzeritem_select(rd_slice_at(m_analyzers, i), select);
    }
}

void AnalyzerDialog::get_analyzers() {
    m_analyzersmodel->clear();
    m_analyzersmodel->setHorizontalHeaderLabels({"Name", "Order"});
    m_analyzers = rd_get_analyzer_plugins(m_context);

    for(usize i = 0; i < rd_slice_length(m_analyzers); i++) {
        const RDAnalyzerPlugin* ap =
            rd_analyzeritem_get_plugin(rd_slice_at(m_analyzers, i));

        auto* nameitem = new QStandardItem(ap->name);
        auto* orderitem = new QStandardItem(QString::number(ap->order, 16));

        nameitem->setCheckable(true);
        nameitem->setCheckState(ap->flags & RD_AF_SELECTED ? Qt::Checked
                                                           : Qt::Unchecked);

        m_analyzersmodel->appendRow({nameitem, orderitem});
    }

    m_ui.tblanalyzers->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    m_ui.tblanalyzers->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);

    this->set_details_visible(m_ui.chkshowdetails->checkState());
}
