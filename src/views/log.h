#pragma once

#include "ui/logview.h"

class LogView: public QWidget {
    Q_OBJECT

public:
    explicit LogView(QWidget* parent = nullptr);

public Q_SLOTS:
    void clear();
    void log(const QString& s);

private:
    ui::LogView m_ui;
};
