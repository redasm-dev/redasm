#include "dashboard.h"
#include "support/utils.h"
#include <QLabel>
#include <QPixmap>
#include <QPushButton>

DashboardView::DashboardView(QWidget* parent): QWidget(parent) {
    this->setAutoFillBackground(true);

    const QString FLAT_STYLESHEET =
        QString{"QPushButton:hover {"
                "background-color: %1;"
                "}"}
            .arg(this->palette().color(QPalette::Window).darker(125).name());

    this->setStyleSheet(FLAT_STYLESHEET);
}

void DashboardView::make_bordered(QPushButton* pb) const {
    const QString BORDERED_STYLESHEET =
        QString("QPushButton {"
                "border-color: %1;"
                "border-style: solid;"
                "border-width: 1px;"
                "}")
            .arg(this->palette().color(QPalette::Text).name());

    pb->setStyleSheet(BORDERED_STYLESHEET);
}

void DashboardView::apply_logo(QLabel* lbl) const {
    lbl->setPixmap(utils::get_logo());
}
