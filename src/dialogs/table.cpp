#include "table.h"
#include "support/utils.h"
#include <QApplication>
#include <QClipboard>
#include <QSortFilterProxyModel>

TableDialog::TableDialog(QWidget* parent): QDialog{parent}, m_ui{this} {
    this->set_button_box_visible(false);
    m_ui.lbldescription->setVisible(false);

    connect(m_ui.tvtable, &QTreeView::doubleClicked, this,
            &TableDialog::on_table_double_clicked);
    connect(m_ui.tvtable, &QTreeView::clicked, this,
            &TableDialog::on_table_clicked);

    connect(m_ui.lesearch, &QLineEdit::textChanged, this,
            [=](const QString& s) {
                auto* sfmodel =
                    static_cast<QSortFilterProxyModel*>(m_ui.tvtable->model());
                sfmodel->setFilterFixedString(s);
            });

    connect(m_ui.ftbcopy, &FeedbackToolButton::feedback, this,
            &TableDialog::on_copy_feedback);
}

TableDialog::TableDialog(const QString& title, QWidget* parent)
    : TableDialog{parent} {
    this->setWindowTitle(title);
}

QAbstractItemModel* TableDialog::model() const { return m_ui.tvtable->model(); }

QAbstractItemModel* TableDialog::source_model() const {
    return qobject_cast<QSortFilterProxyModel*>(this->model())->sourceModel();
}

void TableDialog::set_stretch_last_column(bool b) { // NOLINT
    m_ui.tvtable->header()->setStretchLastSection(b);
}

void TableDialog::set_description(const QString& descr) { // NOLINT
    m_ui.lbldescription->setText(descr);
    m_ui.lbldescription->setVisible(!descr.isEmpty());
}

void TableDialog::set_model(QAbstractItemModel* m) {
    auto* sfmodel = qobject_cast<QSortFilterProxyModel*>(m);

    if(!sfmodel) {
        sfmodel = new QSortFilterProxyModel(this);
        sfmodel->setSourceModel(m);
        sfmodel->setFilterKeyColumn(-1);
    }

    sfmodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_ui.tvtable->setModel(sfmodel);

    for(int i = 0; i < sfmodel->columnCount() - 1; i++)
        this->resize_column(i, QHeaderView::ResizeToContents);
}

void TableDialog::move_column(int fromidx, int toidx) const {
    m_ui.tvtable->header()->moveSection(fromidx, toidx);
}

void TableDialog::hide_column(int idx) const { m_ui.tvtable->hideColumn(idx); }

void TableDialog::resize_column(int idx, QHeaderView::ResizeMode r) const {
    m_ui.tvtable->header()->setSectionResizeMode(idx, r);
}

void TableDialog::set_button_box_visible(bool b) { // NOLINT
    m_ui.buttonbox->setVisible(b);
}

void TableDialog::set_header_visible(bool b) { // NOLINT
    m_ui.tvtable->setHeaderHidden(!b);
}

void TableDialog::on_table_double_clicked(const QModelIndex& index) {
    auto* sfmodel = static_cast<QSortFilterProxyModel*>(m_ui.tvtable->model());
    Q_EMIT double_clicked(sfmodel->mapToSource(index));
}

void TableDialog::on_table_clicked(const QModelIndex& index) {
    auto* sfmodel = static_cast<QSortFilterProxyModel*>(m_ui.tvtable->model());
    Q_EMIT clicked(sfmodel->mapToSource(index));
}

void TableDialog::on_copy_feedback() { // NOLINT
    QString csv = utils::model_to_csv(this->model(), true);
    if(csv.isEmpty()) return;
    qApp->clipboard()->setText(csv);
}

void TableDialog::closeEvent(QCloseEvent* e) {
    Q_EMIT closed();
    QDialog::closeEvent(e);
}
