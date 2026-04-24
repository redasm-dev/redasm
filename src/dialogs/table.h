#pragma once

#include "ui/tabledialog.h"
#include <QDialog>
#include <QHeaderView>

class TableDialog: public QDialog {
    Q_OBJECT

public:
    explicit TableDialog(QWidget* parent = nullptr);
    explicit TableDialog(const QString& title, QWidget* parent = nullptr);
    [[nodiscard]] QAbstractItemModel* model() const;
    void move_column(int fromidx, int toidx) const;
    void hide_column(int idx) const;
    void resize_column(int idx, QHeaderView::ResizeMode r) const;
    void set_description(const QString& descr);
    void set_model(QAbstractItemModel* m);
    void set_button_box_visible(bool b);
    void set_search_box_visible(bool b);
    void set_header_visible(bool b);

private Q_SLOTS:
    void on_table_double_clicked(const QModelIndex& index);
    void on_table_clicked(const QModelIndex& index);

Q_SIGNALS:
    void double_clicked(const QModelIndex& index);
    void clicked(const QModelIndex& index);

private:
    ui::TableDialog m_ui;
};
