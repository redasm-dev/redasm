#pragma once

#include <QFileInfo>
#include <QStyledItemDelegate>

class RecentFilesDelegate: public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit RecentFilesDelegate(QObject* parent);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const override;

private:
    void draw_icon(QPainter* painter, const QStyleOptionViewItem& option) const;
    void draw_filename(QPainter* painter, const QFileInfo& fi,
                       const QStyleOptionViewItem& option) const;
    void draw_filepath(QPainter* painter, const QFileInfo& fi,
                       const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;
};
