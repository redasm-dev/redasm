#pragma once

#include "models/editable.h"
#include <QTreeView>

class EditableListView: public QTreeView {
    Q_OBJECT

public:
    explicit EditableListView(QWidget* parent = nullptr);
    void setModel(QAbstractItemModel* model) override;
    [[nodiscard]] QAbstractItemModel* source_model() const;

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    EditableProxyModel* m_editablemodel{nullptr};
};
