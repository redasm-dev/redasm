#pragma once

#include "ui/aboutdialog.h"

class AboutDialog: public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);

private:
    ui::AboutDialog m_ui;
};
