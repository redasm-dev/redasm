#pragma once

#include <QStyledItemDelegate>
#include <redasm/redasm.h>

class EditableDelegate: public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit EditableDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};
