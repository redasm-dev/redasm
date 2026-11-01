#include "editable.h"
#include "models/editable.h"
#include "support/fontawesome.h"
#include "support/themeprovider.h"
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

EditableDelegate::EditableDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

void EditableDelegate::paint(QPainter* painter,
                             const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    int real_rows = index.model()->rowCount() - 1;
    int real_cols = index.model()->columnCount() - 1;

    if(index.column() == real_cols && index.row() < real_rows) {
        QStyleOptionButton button_option;
        button_option.rect = option.rect;
        button_option.icon =
            FA_ICON_COLOR(0xf00d, theme_provider::color(RD_THEME_FAIL));
        button_option.iconSize = {button_option.rect.height(),
                                  button_option.rect.height()};
        button_option.state = QStyle::State_Enabled;

        if(index.data(EditableProxyModel::BUTTON_PRESSED_ROLE).toBool())
            button_option.state |= QStyle::State_Sunken;
        else
            button_option.state |= QStyle::State_Raised;

        QApplication::style()->drawControl(QStyle::CE_PushButton,
                                           &button_option, painter);
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
