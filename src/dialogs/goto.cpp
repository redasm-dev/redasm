#include "goto.h"

GotoDialog::GotoDialog(RDContext* ctx, QWidget* parent)
    : QDialog{parent}, m_ui{this}, m_context{ctx} {

    m_symbolsmodel = new SymbolsModel(ctx, true, m_ui.tvsymbols);
    m_symbolsmodel->set_highlight_symbol(true);

    m_filtermodel = new QSortFilterProxyModel(m_ui.tvsymbols);
    m_filtermodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filtermodel->setFilterKeyColumn(-1);
    m_filtermodel->setSourceModel(m_symbolsmodel);
    m_ui.tvsymbols->setModel(m_filtermodel);

    connect(m_ui.lesearch, &QLineEdit::textChanged, this,
            [=](const QString&) { this->validate_and_filter_entry(); });
    connect(m_ui.lesearch, &QLineEdit::returnPressed, this,
            &GotoDialog::on_goto_clicked);
    connect(m_ui.tvsymbols, &QTreeView::doubleClicked, this,
            &GotoDialog::on_item_selected);
    connect(m_ui.tvsymbols, &QTreeView::doubleClicked, this,
            &GotoDialog::accept);
    connect(m_ui.pbgoto, &QPushButton::clicked, this,
            &GotoDialog::on_goto_clicked);
}

bool GotoDialog::check_valid_address() {
    if(m_filtermodel->rowCount() == 1) {
        QModelIndex srcindex =
            m_filtermodel->mapToSource(m_filtermodel->index(0, 0));

        if(srcindex.isValid()) {
            this->address = m_symbolsmodel->address(srcindex);
            m_validaddress = true;
        }
    }

    return m_validaddress && rd_is_address(m_context, this->address);
}

void GotoDialog::validate_and_filter_entry() {
    bool ok = false;
    QString s = m_ui.lesearch->text();

    if(s.isEmpty()) {
        m_validaddress = false;

        m_ui.pbgoto->setEnabled(false);
        m_filtermodel->setFilterFixedString(QString{});
        return;
    }

    this->address = s.toULongLong(&ok, 16);
    m_ui.pbgoto->setEnabled(ok);
    m_validaddress = ok;
    m_filtermodel->setFilterFixedString(s);
}

void GotoDialog::on_item_selected(const QModelIndex& index) {
    QModelIndex srcindex = m_filtermodel->mapToSource(index);
    if(!srcindex.isValid()) return;

    this->address = m_symbolsmodel->address(srcindex);
    this->accept();
}

void GotoDialog::on_goto_clicked() {
    if(this->check_valid_address())
        this->accept();
    else
        this->reject();
}
