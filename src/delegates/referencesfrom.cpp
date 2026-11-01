#include "referencesfrom.h"
#include "models/referencesfrom.h"
#include "support/utils.h"
#include <QAbstractProxyModel>
#include <QComboBox>
#include <QLineEdit>

ReferencesFromDelegate::ReferencesFromDelegate(const RDContext* ctx,
                                               QObject* parent)
    : EditableDelegate(parent), m_context{ctx} {}

QWidget*
ReferencesFromDelegate::createEditor(QWidget* parent,
                                     const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {
    Q_UNUSED(option);

    if(index.column() == 0) {
        auto* e = new QLineEdit(parent);
        e->setAlignment(Qt::AlignRight);
        utils::configure_hex_input(e);
        return e;
    }

    if(index.column() == 3) {
        auto* e = new QComboBox(parent);
        e->addItem("READ", RD_DR_READ);
        e->addItem("WRITE", RD_DR_WRITE);
        e->addItem("ADDRESS", RD_DR_ADDRESS);
        e->addItem("CALL", RD_CR_CALL);
        e->addItem("JUMP", RD_CR_JUMP);
        return e;
    }

    return EditableDelegate::createEditor(parent, option, index);
}

void ReferencesFromDelegate::setEditorData(QWidget* editor,
                                           const QModelIndex& index) const {
    const QAbstractItemModel* base = index.model();
    while(const auto* proxy = qobject_cast<const QAbstractProxyModel*>(base))
        base = proxy->sourceModel();

    const auto* m = qobject_cast<const ReferencesFromModel*>(base);
    if(!m) return;

    RDXRef xref = m->xref(index);

    if(index.column() == 0) {
        QString hex_str = utils::to_hex(xref.address, m_context);
        static_cast<QLineEdit*>(editor)->setText(hex_str);
    }
    else if(index.column() == 3) {
        auto* cb = static_cast<QComboBox*>(editor);
        cb->setCurrentIndex(cb->findData(xref.type));
    }
    else {
        EditableDelegate::setEditorData(editor, index);
    }
}

void ReferencesFromDelegate::setModelData(QWidget* editor,
                                          QAbstractItemModel* model,
                                          const QModelIndex& index) const {
    if(index.column() == 0) {
        auto* le = static_cast<QLineEdit*>(editor);
        bool ok = false;
        quint64 val = le->text().toULongLong(&ok, 16);
        if(ok) model->setData(index, val);
    }
    else if(index.column() == 3) {
        auto* cb = static_cast<QComboBox*>(editor);
        model->setData(index, cb->currentData());
    }
    else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void ReferencesFromDelegate::updateEditorGeometry(
    QWidget* editor, const QStyleOptionViewItem& option,
    const QModelIndex& index) const {
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}
