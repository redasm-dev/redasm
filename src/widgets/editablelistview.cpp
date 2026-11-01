#include "editablelistview.h"
#include "delegates/editable.h"
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>

EditableListView::EditableListView(QWidget* parent): QTreeView{parent} {
    this->setUniformRowHeights(true);
    this->setRootIsDecorated(false);
    this->setMouseTracking(true);

    m_editablemodel = new EditableProxyModel(this);
    this->setItemDelegate(new EditableDelegate(this));
}

void EditableListView::setModel(QAbstractItemModel* model) {
    if(!model) {
        QTreeView::setModel(nullptr);
        return;
    }

    m_editablemodel->setSourceModel(model);
    QTreeView::setModel(m_editablemodel);

    this->header()->setStretchLastSection(false);
    this->header()->setSectionResizeMode(m_editablemodel->columnCount({}) - 1,
                                         QHeaderView::ResizeToContents);
}

QAbstractItemModel* EditableListView::source_model() const {
    return m_editablemodel->sourceModel();
}

void EditableListView::mouseDoubleClickEvent(QMouseEvent* event) {
    QModelIndex index = this->indexAt(event->pos());
    int last_row = this->model()->rowCount() - 1;

    if(index.isValid() && index.row() == last_row) {
        int new_row = index.row();

        if(this->model()->insertRow(new_row)) {
            QModelIndex new_index = model()->index(new_row, 0);
            this->setCurrentIndex(new_index);
            this->edit(new_index);
        }

        return;
    }

    QTreeView::mouseDoubleClickEvent(event);
}

void EditableListView::mousePressEvent(QMouseEvent* event) {
    QModelIndex index = this->indexAt(event->pos());
    int real_rows = this->model()->rowCount() - 1;
    int last_column = this->model()->columnCount() - 1;

    if(event->button() == Qt::LeftButton &&
       (index.isValid() && index.column() == last_column &&
        index.row() < real_rows)) {
        this->model()->setData(index, true,
                               EditableProxyModel::BUTTON_PRESSED_ROLE);
        return;
    }

    QTreeView::mousePressEvent(event);
}

void EditableListView::mouseReleaseEvent(QMouseEvent* event) {
    QModelIndex pressed_index = m_editablemodel->pressed_index();
    QModelIndex index = this->indexAt(event->pos());
    int last_column = this->model()->columnCount() - 1;

    if(pressed_index.isValid()) {
        int row_to_delete = pressed_index.row();

        this->model()->setData(pressed_index, false,
                               EditableProxyModel::BUTTON_PRESSED_ROLE);

        if(index.isValid() && index.column() == last_column &&
           index.row() == row_to_delete) {
            this->model()->removeRow(row_to_delete);
            return;
        }
    }

    QTreeView::mouseReleaseEvent(event);
}

void EditableListView::mouseMoveEvent(QMouseEvent* event) {
    QModelIndex idx = this->indexAt(event->pos());

    if(idx.isValid()) {
        int real_rows = m_editablemodel->rowCount(idx.parent()) - 1;
        int real_cols = m_editablemodel->columnCount(idx.parent()) - 1;

        if(idx.column() == real_cols && idx.row() < real_rows) {
            this->setCursor(Qt::PointingHandCursor);
            QTreeView::mouseMoveEvent(event);
            return;
        }
    }

    this->setCursor(Qt::ArrowCursor);
    QTreeView::mouseMoveEvent(event);
}

void EditableListView::leaveEvent(QEvent* event) {
    this->setCursor(Qt::ArrowCursor);
    QTreeView::leaveEvent(event);
}
