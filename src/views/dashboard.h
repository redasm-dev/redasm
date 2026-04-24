#pragma once

#include <QWidget>

class QPushButton;
class QLabel;

class DashboardView: public QWidget {
    Q_OBJECT

public:
    explicit DashboardView(QWidget* parent = nullptr);

protected:
    void make_bordered(QPushButton* pb) const;
    void apply_logo(QLabel* lbl) const;
};
