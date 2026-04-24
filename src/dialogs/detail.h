#pragma once

#include "ui/detaildialog.h"
#include <QDialog>

class DetailDialog: public QDialog {
    Q_OBJECT

public:
    explicit DetailDialog(QWidget* parent = nullptr);
    void set_html(const QString& s);

private:
    ui::DetailDialog m_ui;
};
