#pragma once

#include "delegates/editable.h"
#include <redasm/redasm.h>

class ReferencesFromDelegate: public EditableDelegate {
    Q_OBJECT

public:
    explicit ReferencesFromDelegate(const RDContext* ctx,
                                    QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;
    void setEditorData(QWidget* editor,
                       const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;

private:
    const RDContext* m_context;
};
