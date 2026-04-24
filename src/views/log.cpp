#include "log.h"
#include <QVBoxLayout>

LogView::LogView(QWidget* parent): QWidget{parent}, m_ui{this} {
    this->setVisible(false);
}

void LogView::clear() {
    this->setVisible(false);
    m_ui.ptelogs->clear();
}

void LogView::log(const QString& s) {
    m_ui.ptelogs->insertPlainText(s);
    m_ui.ptelogs->insertPlainText("\n");
    this->setVisible(true);
}
